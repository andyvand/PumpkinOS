package com.pit.pit;

import android.content.Context;
import android.content.pm.PackageManager;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.os.Handler;
import android.os.Looper;

/**
 * Selects and opens a MIDI output device for PumpkinOS.
 *
 * SndPlaySmf melodies are sequenced natively (aaudio.c). When a MIDI device
 * with at least one input port (a port the app can send to: a USB or Bluetooth
 * synthesizer, or a virtual synth app) is present, this class opens it through
 * MidiManager and hands the MidiDevice to the native AMidi provider
 * (amidi.c) with Pumpkin.pitSetMidiDevice(). Without such a device the native
 * side falls back to its built-in software synthesizer.
 *
 * Devices that appear or disappear while the OS is running are handled by the
 * MidiManager device callback.
 */
public class PumpkinMidi {
    private static final String TAG = "PumpkinMidi";

    private final Pumpkin pumpkin;
    private final Handler handler;
    private MidiManager manager;
    private MidiManager.DeviceCallback callback;
    private MidiDevice device;
    private MidiDeviceInfo deviceInfo;
    private boolean opening;
    private boolean running;

    public PumpkinMidi(Pumpkin pumpkin) {
        this.pumpkin = pumpkin;
        this.handler = new Handler(Looper.getMainLooper());
    }

    public void start() {
        handler.post(() -> {
            if (running) return;
            running = true;
            if (!pumpkin.getPackageManager().hasSystemFeature(PackageManager.FEATURE_MIDI)) {
                PumpkinLog.log(PumpkinLog.INFO, TAG, "no MIDI support on this device");
                return;
            }
            manager = (MidiManager) pumpkin.getSystemService(Context.MIDI_SERVICE);
            if (manager == null) {
                PumpkinLog.log(PumpkinLog.INFO, TAG, "no MIDI service");
                return;
            }
            callback = new MidiManager.DeviceCallback() {
                @Override
                public void onDeviceAdded(MidiDeviceInfo info) {
                    PumpkinLog.log(PumpkinLog.INFO, TAG, "device added " + describe(info));
                    if (device == null && !opening) openBest();
                }

                @Override
                public void onDeviceRemoved(MidiDeviceInfo info) {
                    PumpkinLog.log(PumpkinLog.INFO, TAG, "device removed " + describe(info));
                    if (deviceInfo != null && deviceInfo.getId() == info.getId()) {
                        closeDevice();
                        openBest();
                    }
                }
            };
            manager.registerDeviceCallback(callback, handler);
            openBest();
        });
    }

    public void stop() {
        handler.post(() -> {
            if (!running) return;
            running = false;
            if (manager != null && callback != null) {
                manager.unregisterDeviceCallback(callback);
                callback = null;
            }
            closeDevice();
            manager = null;
        });
    }

    private static String describe(MidiDeviceInfo info) {
        String name = info.getProperties().getString(MidiDeviceInfo.PROPERTY_NAME);
        return "#" + info.getId() + " \"" + name + "\" type " + info.getType()
                + " in=" + info.getInputPortCount() + " out=" + info.getOutputPortCount();
    }

    // Prefer real hardware (USB, Bluetooth) over virtual devices; require an
    // input port so that we can actually send notes to it.
    private static int score(MidiDeviceInfo info) {
        if (info.getInputPortCount() == 0) return -1;
        switch (info.getType()) {
            case MidiDeviceInfo.TYPE_USB: return 3;
            case MidiDeviceInfo.TYPE_BLUETOOTH: return 2;
            default: return 1;
        }
    }

    @SuppressWarnings("deprecation")
    private void openBest() {
        if (manager == null || !running) return;
        MidiDeviceInfo best = null;
        int bestScore = -1;
        for (MidiDeviceInfo info : manager.getDevices()) {
            PumpkinLog.log(PumpkinLog.INFO, TAG, "found " + describe(info));
            int s = score(info);
            if (s > bestScore) {
                bestScore = s;
                best = info;
            }
        }
        if (best == null) {
            PumpkinLog.log(PumpkinLog.INFO, TAG, "no usable MIDI output device; using built-in synth");
            return;
        }
        final MidiDeviceInfo chosen = best;
        opening = true;
        PumpkinLog.log(PumpkinLog.INFO, TAG, "opening " + describe(chosen));
        manager.openDevice(chosen, dev -> {
            opening = false;
            if (dev == null) {
                PumpkinLog.log(PumpkinLog.ERROR, TAG, "could not open " + describe(chosen));
                return;
            }
            if (!running || device != null) {
                // stopped or another device won the race in the meantime
                try { dev.close(); } catch (Exception ignored) { }
                return;
            }
            device = dev;
            deviceInfo = chosen;
            pumpkin.pitSetMidiDevice(dev);
            PumpkinLog.log(PumpkinLog.INFO, TAG, "connected " + describe(chosen));
        }, handler);
    }

    private void closeDevice() {
        if (device != null) {
            // detach the native AMidi handles before closing the Java object
            pumpkin.pitSetMidiDevice(null);
            try {
                device.close();
            } catch (Exception ex) {
                PumpkinLog.log(PumpkinLog.ERROR, TAG, "close error " + ex.getMessage());
            }
            device = null;
            deviceInfo = null;
        }
    }
}
