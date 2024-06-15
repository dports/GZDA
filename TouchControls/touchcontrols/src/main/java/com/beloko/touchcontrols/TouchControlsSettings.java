package com.beloko.touchcontrols;


import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.Spinner;


public class TouchControlsSettings {

	static Activity activity;
	static ControlInterface quakeIf;


	static int alpha,fwdSens,strafeSens,pitchSens,yawSens;

	static boolean mouseMode,showSticks,enableWeaponWheel;
	static boolean invertLook,precisionShoot;

	static int doubleTapMove,doubleTapLook;

	public static void setup(Activity a,ControlInterface qif)
	{
		activity = a;
		quakeIf = qif;

		//AssetFileAccess.init(a);
		//AssetFileAccess.setAssetManager(a.getAssets());
	}

	public static void showSettings()
	{
		Log.d("settings","showSettings");

		activity.runOnUiThread(new Runnable(){
			public void run() {
				final Dialog dialog = new Dialog(activity);
				dialog.setContentView(R.layout.touch_controls_settings);
				dialog.setTitle("Touch Control Sensitivity Settings");
				dialog.setCancelable(true);

				final SeekBar alphaSeek = (SeekBar)dialog.findViewById(R.id.alpha_seekbar);
				final SeekBar fwdSeek = (SeekBar)dialog.findViewById(R.id.fwd_seekbar);
				final SeekBar strafeSeek = (SeekBar)dialog.findViewById(R.id.strafe_seekbar);
				final SeekBar pitchSeek = (SeekBar)dialog.findViewById(R.id.pitch_seekbar);
				final SeekBar yawSeek = (SeekBar)dialog.findViewById(R.id.yaw_seekbar);

				final CheckBox mouseModeCheck =  (CheckBox)dialog.findViewById(R.id.mouse_turn_checkbox);

				final CheckBox invertLookCheckBox =  (CheckBox)dialog.findViewById(R.id.invert_loop_checkbox);
				final CheckBox precisionShootCheckBox =  (CheckBox)dialog.findViewById(R.id.precision_shoot_checkbox);
				final CheckBox showSticksCheckBox =  (CheckBox)dialog.findViewById(R.id.show_sticks_checkbox);
				final CheckBox enableWeaponWheelCheckBox =  (CheckBox)dialog.findViewById(R.id.enable_weapon_wheel_checkbox);

				/*
				//Hide controls for lookup/down
				if (Settings.game == IDGame.Doom)
				{
					//pitchSeek.setVisibility(View.GONE);
					invertLookCheckBox.setVisibility(View.GONE);
				}
				 */
				Button add_rem_button = (Button)dialog.findViewById(R.id.add_remove_button);
				add_rem_button.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						TouchControlsEditing.show(activity);
					}
				});

				alphaSeek.setProgress(alpha);
				fwdSeek.setProgress(fwdSens);
				strafeSeek.setProgress(strafeSens);
				pitchSeek.setProgress(pitchSens);
				yawSeek.setProgress(yawSens);

				mouseModeCheck.setChecked(mouseMode);
				invertLookCheckBox.setChecked(invertLook);
				precisionShootCheckBox.setChecked(precisionShoot);
				showSticksCheckBox.setChecked(showSticks);
				enableWeaponWheelCheckBox.setChecked(enableWeaponWheel);

				Spinner move_spinner = (Spinner) dialog.findViewById(R.id.move_dbl_tap_spinner);
				ArrayAdapter<CharSequence> adapterm;

				adapterm	= ArrayAdapter.createFromResource(activity,
						R.array.double_tap_actions, android.R.layout.simple_spinner_item);

				adapterm.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
				move_spinner.setAdapter(adapterm);
				move_spinner.setSelection(doubleTapMove);

				move_spinner.setOnItemSelectedListener(new OnItemSelectedListener() {
					@Override
					public void onItemSelected(AdapterView<?> parent, View view, 
							int pos, long id) {
						doubleTapMove = pos;
					}

					@Override
					public void onNothingSelected(AdapterView<?> arg0) {
						// TODO Auto-generated method stub

					}
				});

				Spinner look_spinner = (Spinner) dialog.findViewById(R.id.look_dbl_tap_spinner);
				ArrayAdapter<CharSequence> adapterl = ArrayAdapter.createFromResource(activity,
						R.array.double_tap_actions, android.R.layout.simple_spinner_item);
				adapterl.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
				look_spinner.setAdapter(adapterl);

				look_spinner.setSelection(doubleTapLook);

				look_spinner.setOnItemSelectedListener(new OnItemSelectedListener() {
					@Override
					public void onItemSelected(AdapterView<?> parent, View view, 
							int pos, long id) {
						doubleTapLook = pos;
					}

					@Override
					public void onNothingSelected(AdapterView<?> arg0) {
						// TODO Auto-generated method stub

					}
				});

				dialog.setOnDismissListener(new OnDismissListener() {

					@Override
					public void onDismiss(DialogInterface dialog) {
						alpha = alphaSeek.getProgress();
						fwdSens = fwdSeek.getProgress();
						strafeSens = strafeSeek.getProgress();
						pitchSens = pitchSeek.getProgress();
						yawSens = yawSeek.getProgress();

						mouseMode = mouseModeCheck.isChecked();
						invertLook = invertLookCheckBox.isChecked();
						precisionShoot = precisionShootCheckBox.isChecked();
						showSticks = showSticksCheckBox.isChecked();
						enableWeaponWheel = enableWeaponWheelCheckBox.isChecked();

						saveSettings(activity);
						sendToQuake();
					}
				});

				Button save = (Button)dialog.findViewById(R.id.save_button);
				save.setOnClickListener(new OnClickListener() {

					@Override
					public void onClick(View v) {
						alpha = alphaSeek.getProgress();
						fwdSens = fwdSeek.getProgress();
						strafeSens = strafeSeek.getProgress();
						pitchSens = pitchSeek.getProgress();
						yawSens = yawSeek.getProgress();

						mouseMode = mouseModeCheck.isChecked();
						invertLook = invertLookCheckBox.isChecked();
						precisionShoot = precisionShootCheckBox.isChecked();
						showSticks = showSticksCheckBox.isChecked();
						enableWeaponWheel = enableWeaponWheelCheckBox.isChecked();

						saveSettings(activity);
						sendToQuake();
						dialog.dismiss();
					}
				});

				Button cancel = (Button)dialog.findViewById(R.id.cancel_button);
				cancel.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						dialog.dismiss();
					}
				});

				dialog.show();
			}
		});

	}



	public static void sendToQuake()
	{

		int other = 0;
		other += mouseMode?0x2:0;
		other += invertLook?0x4:0;
		other += precisionShoot?0x8:0;

		other += (doubleTapMove << 4) & 0xF0;
		other += (doubleTapLook << 8) & 0xF00;

		other += showSticks?0x1000:0;
		other += enableWeaponWheel?0x2000:0;

		other += TouchSettings.hideTouchControls?0x80000000:0;

		quakeIf.setTouchSettings_if(
				(float)alpha/(float)100,
				(strafeSens)/(float)50, 
				(fwdSens)/(float)50, 
				(pitchSens)/(float)50, 
				(yawSens)/(float)50, 
				other);
	}

	public static void loadSettings(Context ctx)
	{
		alpha = TouchSettings.getIntOption(ctx, "alpha", 50);
		fwdSens = TouchSettings.getIntOption(ctx, "fwdSens", 50);
		strafeSens = TouchSettings.getIntOption(ctx, "strafeSens", 50);
		pitchSens = TouchSettings.getIntOption(ctx, "pitchSens", 50);
		yawSens = TouchSettings.getIntOption(ctx, "yawSens", 50);

		mouseMode = TouchSettings.getBoolOption(ctx, "mouse_mode", true);
		invertLook = TouchSettings.getBoolOption(ctx, "invert_look", false);
		precisionShoot = TouchSettings.getBoolOption(ctx, "precision_shoot", false);
		showSticks = TouchSettings.getBoolOption(ctx, "show_sticks", false);
		enableWeaponWheel =  TouchSettings.getBoolOption(ctx, "enable_ww", true);

		doubleTapMove  = TouchSettings.getIntOption(ctx, "double_tap_move", 0);
		doubleTapLook  = TouchSettings.getIntOption(ctx, "double_tap_look", 0);
	}

	public static void saveSettings(Context ctx)
	{
		TouchSettings.setIntOption(ctx, "alpha", alpha);
		TouchSettings.setIntOption(ctx, "fwdSens", fwdSens);
		TouchSettings.setIntOption(ctx, "strafeSens", strafeSens);
		TouchSettings.setIntOption(ctx, "pitchSens", pitchSens);
		TouchSettings.setIntOption(ctx, "yawSens", yawSens);

		TouchSettings.setBoolOption(ctx, "invert_look", invertLook);
		TouchSettings.setBoolOption(ctx, "precision_shoot", precisionShoot);
		TouchSettings.setBoolOption(ctx, "show_sticks", showSticks);
		TouchSettings.setBoolOption(ctx, "enable_ww", enableWeaponWheel);

		TouchSettings.setIntOption(ctx, "double_tap_move", doubleTapMove);
		TouchSettings.setIntOption(ctx, "double_tap_look", doubleTapLook);

	}
}
