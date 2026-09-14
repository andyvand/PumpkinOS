package com.pit.pit;

public interface PumpkinUpdate {
    /**
     * Periodic housekeeping callback while the app is in the foreground.
     * @param finish true once the native PumpkinOS thread has exited.
     */
    void pumpkinTick(boolean finish);
}
