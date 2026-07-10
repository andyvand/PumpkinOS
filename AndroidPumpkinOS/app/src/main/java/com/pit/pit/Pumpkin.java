package com.pit.pit;

import android.app.Application;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Pumpkin extends Application {

    static {
        System.loadLibrary("native-lib");
    }

    private int pe = -1;
    // These flags are written on the native "pumpkin" render thread and read on
    // the main/UI thread (and vice-versa); without volatile the UI thread may
    // never observe the render thread setting exited=true (so the activity never
    // finishes) or see a stale paused/on value.
    private volatile boolean on;
    private volatile boolean paused;
    private volatile boolean exited;
    private Handler handler;
    private Runnable r;
    private PumpkinUpdate updater;
    private ExecutorService exec;

    @Override
    public void onCreate() {
        super.onCreate();
        PumpkinLog.log(PumpkinLog.INFO, "Application", "onCreate");

        on = false;
        paused = true;
        exited = false;
        handler = new Handler(Looper.getMainLooper());
        installFiles();

        r = new Runnable() {
            public void run() {
                if (updater != null && !paused) {
                    updater.updateDisplay(exited);
                    if (!exited) handler.postDelayed(this, 100);
                }
            }
        };
    }

    public void start(Bitmap bitmap) {
        Runnable r = () -> {
            PumpkinLog.log(PumpkinLog.INFO, "Application", "pumpkin thread begin");
            // Mark the OS as running so MainActivity's onPause/onResume actually
            // forward pitPause() to the native side. Without this pumpkinOn()
            // is always false, so the native OS is never paused when backgrounded
            // nor refreshed on return, which left a frozen/stale frame on resume.
            pumpkinSetOn(true);
            pitUpdate(bitmap);
            pe = pitInit();
            if (pe != -1) pitFinish(pe);
            pumpkinSetOn(false);
            exited = true;
            PumpkinLog.log(PumpkinLog.INFO, "Application", "pumpkin thread end");
        };
        PumpkinLog.log(PumpkinLog.INFO, "Application", "start");
        // A previous native session may still be tearing down: stop() only
        // requests exit and waits a bounded time, so on a fast onStop/onStart
        // cycle the old "pumpkin" thread can still be alive here. Launching a
        // second pitInit() while the first is running re-runs sys_init()/
        // thread_init()/pumpkin_global_init() over shared global state, which
        // corrupts it (garbled drawing, SIGSEGV). Wait for the old thread to
        // finish; if it refuses to, skip this start rather than double-init.
        if (exec != null) {
            exec.shutdown();
            try {
                if (!exec.awaitTermination(3, TimeUnit.SECONDS)) {
                    PumpkinLog.log(PumpkinLog.ERROR, "Application", "previous pumpkin thread still running; skipping start");
                    return;
                }
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
                return;
            }
            exec = null;
        }
        // Fresh boot: clear the "OS thread has exited" flag so a restart (e.g.
        // after background/foreground) isn't immediately treated as finished,
        // which would stop display updates and tear down the activity.
        exited = false;
        // PumpkinOS runs an entire OS (launcher + nested app launches + the Lua
        // script engine + the m68k emulator) on this single native thread. The
        // default JVM/executor thread stack (~1 MB) is far too small for that
        // depth of native recursion; overflowing it silently corrupts memory,
        // which shows up as garbled/black drawing and random SIGSEGVs after an
        // app is launched. The desktop build runs on a main/pthread with a
        // multi-MB stack, which is why it doesn't reproduce there. Give the
        // pumpkin thread an explicit large stack.
        exec = Executors.newSingleThreadExecutor(
                runnable -> new Thread(null, runnable, "pumpkin", 32L * 1024 * 1024));
        exec.execute(r);
    }

    public void stop() {
        PumpkinLog.log(PumpkinLog.INFO, "Application", "stop");
        pumpkinSetOn(false);
        if (!exited) pitRequestFinish();
        if (exec == null) return;
        // shutdown() is required for awaitTermination() to ever return true:
        // without it the call always times out, the native OS thread is never
        // reaped, and the next onStart() stacks a second pitInit() on top of the
        // first — double-initializing all native global state, which crashes.
        exec.shutdown();
        try {
            if (!exec.awaitTermination(2, TimeUnit.SECONDS)) {
                // Native teardown (thread_wait_all + pumpkin_global_finish) can
                // outlast this wait. Leave exec non-null so the next start()
                // waits for this thread instead of launching a concurrent
                // pitInit(); do NOT null it here.
                PumpkinLog.log(PumpkinLog.ERROR, "Application", "stop timeout; deferring teardown to next start");
                return;
            }
        } catch (Exception ex) {
            PumpkinLog.log(PumpkinLog.ERROR, "Application", "stop error " + ex.getMessage());
        }
        exec = null;
    }

    public void pumpkinSetUpdate(PumpkinUpdate updater) {
        this.updater = updater;
    }

    public boolean pumpkinOn() {
        return on;
    }

    public void pumpkinSetOn(boolean on) {
        this.on = on;
    }

    public void pumpkinSetPaused(boolean paused) {
        PumpkinLog.log(PumpkinLog.INFO, "Application", "pumpkinSetPaused " + paused);
        this.paused = paused;
        if (!paused) {
            handler.post(r);
        }
    }

    private void copyFile(int id, File dir, String name) throws IOException {
        if (dir.exists()) {
            File f = new File(dir, name);
            if (!f.exists() && f.createNewFile()) {
                FileOutputStream os = new FileOutputStream(f);
                Resources r = getResources();
                InputStream is = r.openRawResource(id);
                Log.i("Pumpkin", "copying file " + name);
                copyFile(is, os);
            }
        }
    }

    private void copyFile(File from, File to) throws IOException {
        if (to.exists()) {
            if (!to.delete()) {
                Log.e("Pumpkin", "could noe delete file " + to.getName());
            }
        }
        if (to.createNewFile()) {
            FileOutputStream os = new FileOutputStream(to);
            InputStream is = new FileInputStream(from);
            Log.i("Pumpkin", "copying file " + from.getName());
            copyFile(is, os);
        }
    }

    private void copyFile(InputStream is, OutputStream os) throws IOException {
        byte[] buffer = new byte[4096];
        int bytesRead;
        while ((bytesRead = is.read(buffer)) != -1) {
            os.write(buffer, 0, bytesRead);
        }
        is.close();
        os.close();
    }

    private void installFiles() {
        try {
            // getFilesDir: /data/user/0/com.pit.pit/files
            // getDir("storage", Context.MODE_PRIVATE): /data/user/0/com.pit.pit/app_storage
            // files downloaded via Bluetooth: /storage/emulated/0/Download

            getDir("storage", Context.MODE_PRIVATE);
            getDir("card", Context.MODE_PRIVATE);
            getDir("registry", Context.MODE_PRIVATE);
            File dir = getDir("install", Context.MODE_PRIVATE);
            copyFile(R.raw.boot, dir, "BOOT.prc");
            if (getArch() == 9) { // armv7-a
                copyFile(R.raw.launcher_a32, dir, "Launcher.prc");
                copyFile(R.raw.addressbook_a32, dir, "AddressBook.prc");
                copyFile(R.raw.datebook_a32, dir, "DateBook.prc");
                copyFile(R.raw.memopad_a32, dir, "MemoPad.prc");
                copyFile(R.raw.command_a32, dir, "Command.prc");
                copyFile(R.raw.luasyntax_a32, dir, "LuaSyntax.prc");
                copyFile(R.raw.vi_a32, dir, "vi.prc");
                copyFile(R.raw.preferences_a32, dir, "Preferences.prc");
                copyFile(R.raw.todolist_a32, dir, "ToDoList.prc");
                copyFile(R.raw.unicornarm_a32, dir, "UnicornArm.prc");
                copyFile(R.raw.spacetrader_a32, dir, "SpaceTrader.prc");
            } else if (getArch() == 10) { // arm64-v8a
                copyFile(R.raw.launcher_a64, dir, "Launcher.prc");
                copyFile(R.raw.addressbook_a64, dir, "AddressBook.prc");
                copyFile(R.raw.datebook_a64, dir, "DateBook.prc");
                copyFile(R.raw.memopad_a64, dir, "MemoPad.prc");
                copyFile(R.raw.command_a64, dir, "Command.prc");
                copyFile(R.raw.luasyntax_a64, dir, "LuaSyntax.prc");
                copyFile(R.raw.vi_a64, dir, "vi.prc");
                copyFile(R.raw.preferences_a64, dir, "Preferences.prc");
                copyFile(R.raw.todolist_a64, dir, "ToDoList.prc");
                copyFile(R.raw.unicornarm_a64, dir, "UnicornArm.prc");
                copyFile(R.raw.spacetrader_a64, dir, "SpaceTrader.prc");
            } else if (getArch() == 17) { // x86
                copyFile(R.raw.launcher_i32, dir, "Launcher.prc");
                copyFile(R.raw.addressbook_i32, dir, "AddressBook.prc");
                copyFile(R.raw.datebook_i32, dir, "DateBook.prc");
                copyFile(R.raw.memopad_i32, dir, "MemoPad.prc");
                copyFile(R.raw.command_i32, dir, "Command.prc");
                copyFile(R.raw.luasyntax_i32, dir, "LuaSyntax.prc");
                copyFile(R.raw.vi_i32, dir, "vi.prc");
                copyFile(R.raw.preferences_i32, dir, "Preferences.prc");
                copyFile(R.raw.todolist_i32, dir, "ToDoList.prc");
                copyFile(R.raw.unicornarm_i32, dir, "UnicornArm.prc");
                copyFile(R.raw.spacetrader_i32, dir, "SpaceTrader.prc");
            } else if (getArch() == 18) { // x86_64
                copyFile(R.raw.launcher_i64, dir, "Launcher.prc");
                copyFile(R.raw.addressbook_i64, dir, "AddressBook.prc");
                copyFile(R.raw.datebook_i64, dir, "DateBook.prc");
                copyFile(R.raw.memopad_i64, dir, "MemoPad.prc");
                copyFile(R.raw.command_i64, dir, "Command.prc");
                copyFile(R.raw.luasyntax_i64, dir, "LuaSyntax.prc");
                copyFile(R.raw.vi_i64, dir, "vi.prc");
                copyFile(R.raw.preferences_i64, dir, "Preferences.prc");
                copyFile(R.raw.todolist_i64, dir, "ToDoList.prc");
                copyFile(R.raw.unicornarm_i64, dir, "UnicornArm.prc");
                copyFile(R.raw.spacetrader_i64, dir, "SpaceTrader.prc");
            } else if (getArch() == 26) { // riscv64
                copyFile(R.raw.launcher_r64, dir, "Launcher.prc");
                copyFile(R.raw.addressbook_r64, dir, "AddressBook.prc");
                copyFile(R.raw.datebook_r64, dir, "DateBook.prc");
                copyFile(R.raw.memopad_r64, dir, "MemoPad.prc");
                copyFile(R.raw.command_r64, dir, "Command.prc");
                copyFile(R.raw.luasyntax_r64, dir, "LuaSyntax.prc");
                copyFile(R.raw.vi_r64, dir, "vi.prc");
                copyFile(R.raw.preferences_r64, dir, "Preferences.prc");
                copyFile(R.raw.todolist_r64, dir, "ToDoList.prc");
                copyFile(R.raw.unicornarm_r64, dir, "UnicornArm.prc");
                copyFile(R.raw.spacetrader_r64, dir, "SpaceTrader.prc");
            }
        } catch (Exception ex) {
            Log.e("Pumpkin", Objects.requireNonNull(ex.getMessage()));
        }
    }

    public void installFile(File from, String name) {
        if (name.endsWith(".prc") || name.endsWith(".pdb")) {
            File dir = getDir("install", Context.MODE_PRIVATE);
            File to = new File(dir, name);
            try {
                copyFile(from, to);
                if (pe != -1) {
                    pitDeploy("/app_install/" + name);
                }
            } catch (IOException ex) {
                Log.e("Pumpkin", ex.getMessage());
            }
        }
    }

    // application/vnd.palm

    /*
    private void sendFile(File file) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SEND);
        intent.setType("image/png");
        intent.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(file));

        PackageManager pm = getPackageManager();
        List<ResolveInfo> appsList = pm.queryIntentActivities(intent, 0);
        if (appsList.size() > 0) {
            String packageName = null;
            String className = null;

            for (ResolveInfo info: appsList) {
                packageName = info.activityInfo.packageName;
                if (packageName.equals("com.android.bluetooth")) {
                    className = info.activityInfo.name;
                    break;
                }
            }
            if (className != null) {
                intent.setClassName(packageName, className);
                startActivity(intent);
            }
        }
    }
    */

    private native int pitInit();
    private native void pitFinish(int pe);
    private native void pitDeploy(String path);
    private native void pitRequestFinish();
    public native void pitUpdate(Bitmap bitmap);
    public native void pitPause(boolean paused);
    public native void pitTouch(int action, int x, int y);
    // Held around Canvas.drawBitmap so the UI thread never reads a scanline
    // while the native compositor is writing it (prevents tearing).
    public native void pitLockBitmap();
    public native void pitUnlockBitmap();
    public native void pitSetBattery(int level);
    public native int getArch();
}
