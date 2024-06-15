package net.nullsum.doom;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;

import com.beloko.touchcontrols.TouchSettings;

import java.io.File;
import java.io.IOException;

class AppSettings {

    public static String gzdoomBaseDir;

    public static String graphicsDir = "";

    public static boolean immersionMode;

    public static void resetBaseDir(Context ctx) {
        gzdoomBaseDir = Environment.getExternalStorageDirectory().toString() + "/GZDoom";
        setStringOption(ctx, "base_path", gzdoomBaseDir);
    }

    public static void reloadSettings(Context ctx) {

        TouchSettings.reloadSettings(ctx);

        gzdoomBaseDir = getStringOption(ctx, "base_path", null);
        if (gzdoomBaseDir == null) {
            resetBaseDir(ctx);
        }

        String music = getStringOption(ctx, "music_path", null);
        if (music == null) {
            music = gzdoomBaseDir + "/doom/Music";
            setStringOption(ctx, "music_path", music);
        }

        graphicsDir = ctx.getFilesDir().toString() + "/";

        immersionMode = getBoolOption(ctx, "immersion_mode");
    }

    public static String getQuakeFullDir() {
        String quakeFilesDir = AppSettings.gzdoomBaseDir;
        return quakeFilesDir + "/config";
    }

    public static void createDirectories(Context ctx) {
        boolean scan = false;

        String d = "";
        if (!new File(getQuakeFullDir() + d).exists())
            scan = true;

        new File(getQuakeFullDir() + d).mkdirs();

        //This is totally stupid, need to do this so folder shows up in explorer!
        if (scan) {
            File f = new File(getQuakeFullDir() + d, "temp_");
            try {
                f.createNewFile();
                new SingleMediaScanner(ctx, f.getAbsolutePath());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } else {
            new File(getQuakeFullDir() + d, "temp_").delete();
        }
    }

    private static boolean getBoolOption(Context ctx, String name) {
        SharedPreferences settings = ctx.getSharedPreferences("OPTIONS", Context.MODE_MULTI_PROCESS);
        return settings.getBoolean(name, true);
    }

    public static int getIntOption(Context ctx, String name, int def) {
        SharedPreferences settings = ctx.getSharedPreferences("OPTIONS", Context.MODE_MULTI_PROCESS);
        return settings.getInt(name, def);
    }

    public static void setIntOption(Context ctx, String name, int value) {
        SharedPreferences settings = ctx.getSharedPreferences("OPTIONS", Context.MODE_MULTI_PROCESS);
        SharedPreferences.Editor editor = settings.edit();
        editor.putInt(name, value);
        editor.apply();
    }

    private static String getStringOption(Context ctx, String name, String def) {
        SharedPreferences settings = ctx.getSharedPreferences("OPTIONS", Context.MODE_MULTI_PROCESS);
        return settings.getString(name, def);
    }

    public static void setStringOption(Context ctx, String name, String value) {
        SharedPreferences settings = ctx.getSharedPreferences("OPTIONS", Context.MODE_MULTI_PROCESS);
        SharedPreferences.Editor editor = settings.edit();
        editor.putString(name, value);
        editor.apply();
    }
}
