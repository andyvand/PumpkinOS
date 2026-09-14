package com.pit.pit;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Display surface for PumpkinOS.
 *
 * The native compositor renders straight into this view's Surface through an
 * ANativeWindow (see window.c): it keeps a shadow framebuffer of the logical
 * 320x544 screen and posts it whenever PumpkinOS finishes a frame. The system
 * compositor scales that buffer to the view bounds in hardware. Nothing is
 * drawn from Java, so there is no Bitmap, no onDraw and no invalidate timer.
 *
 * This view only forwards the Surface lifecycle to native code and maps touch
 * coordinates from view pixels back to logical screen pixels.
 */
public class PumpkinSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private static final int DIA_HEIGHT = 160;
    private static final int BUTTON_HEIGHT = 64;

    public PumpkinSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }

    public int getScreenWidth() {
        return 320;
    }

    public int getScreenHeight() {
        return 320;
    }

    private int getLogicalHeight() {
        return getScreenHeight() + DIA_HEIGHT + BUTTON_HEIGHT;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        PumpkinLog.log(PumpkinLog.INFO, "PumpkinSurfaceView", "surfaceCreated");
        getPumpkin().pitSetSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // The native side fixes the buffer geometry at the logical screen size
        // and lets the compositor scale, so a view resize needs no action here.
        // Touch mapping reads the current view size on every event.
        PumpkinLog.log(PumpkinLog.INFO, "PumpkinSurfaceView", "surfaceChanged " + width + "x" + height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        PumpkinLog.log(PumpkinLog.INFO, "PumpkinSurfaceView", "surfaceDestroyed");
        // Synchronous: once this returns the native side holds no reference to
        // the Surface, as SurfaceHolder.Callback requires.
        getPumpkin().pitSetSurface(null);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        int vw = getWidth();
        int vh = getHeight();
        if (vw <= 0 || vh <= 0) return true;

        int x = (int)(event.getX() * getScreenWidth() / vw);
        int y = (int)(event.getY() * getLogicalHeight() / vh);

        switch (action) {
            case MotionEvent.ACTION_DOWN:
                getPumpkin().pitTouch(0, x, y);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                getPumpkin().pitTouch(1, x, y);
                break;
            case MotionEvent.ACTION_MOVE:
                getPumpkin().pitTouch(2, x, y);
                break;
        }

        return true;
    }

    private Activity getActivity() {
        Context context = getContext();
        while (context instanceof ContextWrapper) {
            if (context instanceof Activity) {
                return (Activity)context;
            }
            context = ((ContextWrapper)context).getBaseContext();
        }
        return null;
    }

    private Pumpkin getPumpkin() {
        Activity activity = getActivity();
        return activity != null ? (Pumpkin)activity.getApplication() : (Pumpkin)getContext().getApplicationContext();
    }
}
