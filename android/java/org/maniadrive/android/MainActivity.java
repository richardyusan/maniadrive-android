/*
 * ManiaDrive Android port - activity.
 * The game itself is native code (NativeActivity, lib "maniadrive").
 * This class only adds: immersive full screen, the soft keyboard for the
 * player name / server fields, and a Wi-Fi multicast lock for LAN games.
 * GPL v2 or later, like ManiaDrive.
 */
package org.maniadrive.android;

import android.app.NativeActivity;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

public class MainActivity extends NativeActivity {

    static {
        // load it here too, so the JNI method below can be resolved
        System.loadLibrary("maniadrive");
    }

    /** a character typed on the (soft) keyboard */
    static native void nativeChar(int ch);

    private WifiManager.MulticastLock multicastLock;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if (Build.VERSION.SDK_INT >= 28) {
            getWindow().getAttributes().layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }
        hideSystemBars();
        try {
            WifiManager wifi = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            if (wifi != null) {
                multicastLock = wifi.createMulticastLock("maniadrive-lan");
                multicastLock.setReferenceCounted(false);
                multicastLock.acquire();
            }
        } catch (Exception e) {
            multicastLock = null;
        }
    }

    @Override
    protected void onDestroy() {
        if (multicastLock != null) multicastLock.release();
        super.onDestroy();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) hideSystemBars();
    }

    @SuppressWarnings("deprecation")
    private void hideSystemBars() {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_FULLSCREEN
            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

    /** called from native code (game thread) */
    public void showKeyboard(boolean show) {
        runOnUiThread(new KeyboardTask(this, show));
    }

    /* named class instead of an anonymous one (d8 of build-tools 34 chokes on
       anonymous classes compiled by recent JDKs) */
    static final class KeyboardTask implements Runnable {
        private final MainActivity activity;
        private final boolean show;

        KeyboardTask(MainActivity activity, boolean show) {
            this.activity = activity;
            this.show = show;
        }

        @Override
        public void run() {
            InputMethodManager imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            View v = activity.getWindow().getDecorView();
            if (imm == null) return;
            if (show) {
                v.setFocusable(true);
                v.setFocusableInTouchMode(true);
                v.requestFocus();
                imm.showSoftInput(v, InputMethodManager.SHOW_FORCED);
            } else {
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                activity.hideSystemBars();
            }
        }
    }

    /* Printable characters are sent to the game here (works for every
       keyboard layout); arrows, Back, Delete and gamepad buttons go through
       the normal NativeActivity input queue. */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int action = event.getAction();
        if (action == KeyEvent.ACTION_MULTIPLE && event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN) {
            String chars = event.getCharacters();
            if (chars != null) {
                for (int i = 0; i < chars.length(); i++) nativeChar(chars.charAt(i));
            }
            return true;
        }
        if (!KeyEvent.isGamepadButton(event.getKeyCode())
                && event.getKeyCode() != KeyEvent.KEYCODE_DEL
                && event.getKeyCode() != KeyEvent.KEYCODE_BACK) {
            int uc = event.getUnicodeChar(event.getMetaState());
            if (uc == '\n' || (uc >= 32 && uc < 127)) {
                if (action == KeyEvent.ACTION_DOWN) nativeChar(uc);
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }
}
