/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ru.smoluks.android.bluetoothlegatt;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Toast;

public class DeviceControlActivity extends Activity implements
		OnSeekBarChangeListener {
	private final static String TAG = DeviceControlActivity.class
			.getSimpleName();
	public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
	public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
	public static final String EXTRAS_DEVICE_RSSI = "DEVICE_RSSI";
	private String mDeviceName;
	private String mDeviceAddress;
	private String mDeviceRssi;
	private BluetoothLeService mBluetoothLeService;
	Bitmap colorbar;
	private boolean mConnected = false;
	private BluetoothGattCharacteristic myreadchar;
	private BluetoothGattCharacteristic mywritechar;
	SeekBar LightBar;
	SeekBar LedColor;
	ListView lv;

	private Handler timerHandler = new Handler();
	volatile private int wcommand = 0;
	volatile private int icommand = 0;

	Deque<byte[]> stack;
	private boolean execflag = false;
	private byte currcommand = 0;

	// ----------------------событи€ формы---------------------
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.gatt_services_characteristics);
		final Intent intent = getIntent();
		mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
		mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
		mDeviceRssi = intent.getStringExtra(EXTRAS_DEVICE_RSSI);
		getActionBar().setTitle(mDeviceName + " " + mDeviceRssi + "dbm");
		getActionBar().setDisplayHomeAsUpEnabled(true);
		Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
		bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
		LightBar = (SeekBar) findViewById(R.id.lightBar);
		LedColor = (SeekBar) findViewById(R.id.ledcolor);
		LightBar.setOnSeekBarChangeListener(this);
		LedColor.setOnSeekBarChangeListener(this);
		colorbar = BitmapFactory.decodeResource(getResources(),
				R.drawable.colorbar);

		stack = new ArrayDeque<byte[]>();
		lv = (ListView) findViewById(R.id.expandableListView1);		
		lv.setOnItemClickListener(new AdapterView.OnItemClickListener()
		{
			@Override
			public void onItemClick(AdapterView<?> parent, View itemClicked, int position, long id)
			{
				
			}
		});
		
	}
	
	public void onClick(View view)
	{
		showPopupMenu(view);
    }

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
		if (mBluetoothLeService != null) {
			final boolean result = mBluetoothLeService.connect(mDeviceAddress);
			Log.d(TAG, "Connect request result=" + result);
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(mGattUpdateReceiver);
	}

	@Override
	protected void onDestroy() {
		unbindService(mServiceConnection);
		mBluetoothLeService = null;
		super.onDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.gatt_services, menu);
		if (mConnected) {
			menu.findItem(R.id.menu_connect).setVisible(false);
			menu.findItem(R.id.menu_disconnect).setVisible(true);
		} else {
			menu.findItem(R.id.menu_connect).setVisible(true);
			menu.findItem(R.id.menu_disconnect).setVisible(false);
		}
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.menu_connect:
			mBluetoothLeService.connect(mDeviceAddress);
			return true;
		case R.id.menu_disconnect:
			mBluetoothLeService.disconnect();
			return true;
		case android.R.id.home:
			onBackPressed();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	private void showPopupMenu(View v) {
		PopupMenu popupMenu = new PopupMenu(DeviceControlActivity.this, v);
		popupMenu.inflate(R.menu.popupmenu);
		popupMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener()
		{
					@Override
					public boolean onMenuItemClick(MenuItem item)
					{
						switch (item.getItemId())
						{
						case R.id.item1:
							return true;
						case R.id.item2:
							return true;
						case R.id.item3:
							return true;
						default:
							return false;
						}
					}
				});
		popupMenu.setOnDismissListener(new PopupMenu.OnDismissListener()
		{
			@Override
			public void onDismiss(PopupMenu menu)
			{
			}
		});
		popupMenu.show();
	}
	
	// --------------событи€ сервиса и команд------------------------------------
	private final ServiceConnection mServiceConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName componentName,
				IBinder service) {
			mBluetoothLeService = ((BluetoothLeService.LocalBinder) service)
					.getService();
			if (!mBluetoothLeService.initialize()) {
				Log.e(TAG, "Unable to initialize Bluetooth");
				finish();
			}
			mBluetoothLeService.connect(mDeviceAddress);
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
			mBluetoothLeService = null;
		}
	};

	private Runnable timercallback = new Runnable() {
		@Override
		public void run() {
			Log.e("timer", Integer.toString(wcommand));
			synchronized (timerHandler) {
				if (wcommand == 1) {
					mBluetoothLeService.writeCharacteristic(mywritechar);
					timerHandler.postAtTime(timercallback, 5000);
					wcommand = 2;
				} else if (wcommand == 2) {
					wcommand = 0;
					makeresult(null);
				}

			}
		}
	};

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();
			if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
				mConnected = true;
				// updateConnectionState(R.string.connected);
				invalidateOptionsMenu();
			} else if (BluetoothLeService.ACTION_GATT_DISCONNECTED
					.equals(action)) {
				mConnected = false;
				// updateConnectionState(R.string.disconnected);
				invalidateOptionsMenu();
				clearUI();
			} else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED
					.equals(action)) {
				// Show all the supported services and characteristics on the
				// user interface.
				displayGattServices(mBluetoothLeService
						.getSupportedGattServices());
			} else if (BluetoothLeService.ACTION_DATA_WRITED.equals(action)) {
				mBluetoothLeService.readCharacteristic(myreadchar);
			} else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
				synchronized (timerHandler) {
					wcommand = 0;
					timerHandler.removeCallbacks(timercallback);
				}
				byte[] d = intent.getByteArrayExtra("data");
				String s = "0x";
				for (byte b : d) {
					s += "0123456789ABCDEF".charAt((b & 0xF0) >> 4);
					s += "0123456789ABCDEF".charAt(b & 0xF);
				}
				Log.e("answer", s);
				makeresult(d);
			}
		}
	};

	private final void send(byte[] data) {
		if (mBluetoothLeService != null) {
			mywritechar.setValue(data);
			synchronized (timerHandler) {
				if (wcommand == 0) {
					String s = "";
					for (byte b : data)
						s += Byte.toString(b);
					Log.e("send", s);
					wcommand = 1;
					mBluetoothLeService.writeCharacteristic(mywritechar);
					// timerHandler.postAtTime(timercallback, 10000);
				}
			}

		} else
			Log.e("send", "mBluetoothLeService niht");
	}

	// ---------------------непосредственно обработчики и  команды-----------------------------------------
	void startupread()
	{
		byte[] r3 = new byte[] { 0x07 };// кол-во эвентов
		stack.push(r3);
		byte[] r2 = new byte[] { 0x02 };// led
		stack.push(r2);
		byte[] r1 = new byte[] { 0x06 };// €ркость
		stack.push(r1);
		makenextcommand();
	}

	void makenextcommand() {
		if (!execflag) {
			if (stack.size() > 0) {
				// выполнить команду из стека
				execflag = true;
				byte[] b = stack.pop();
				currcommand = b[0];
				send(b);
			} else
				// всЄ выполнилось
				execflag = false;
		}
	}

	void makeresult(byte[] buffer) {
		if (buffer != null) {
			switch (currcommand) {
			case 2:
				// здесь конвертер RGB - положение
				int i = 0;
				int lcolor = 0 + buffer[0] << 24 + buffer[1] << 16 + buffer[2] << 8;
				boolean b = false;
				while (!b && i < colorbar.getWidth()) {
					if (lcolor == (colorbar.getPixel(i, 2) & 0xFFFFFF)) {
						LedColor.setProgress(i * 1000 / colorbar.getWidth());
						b = true;
					}
					i++;
				}
				break;
			case 6:
				// €ркость
				LightBar.setProgress(buffer[0] & 0xFF);
				break;
			case 7:
				// кол-во событий
				break;
			default:
				if ((buffer[0] & 0xFF) != 0xAA)
					errormessage(buffer[0] & 0xFF);
				break;
			}
		}
		execflag = false;
		makenextcommand();
	}

	void errormessage(int b) {
		String s;
		if (b == 0xBB)
			s = "checksum error";
		else if (b == 0xDD)
			s = "dev timeout error";
		else
			s = "unknown error";
		AlertDialog.Builder builder = new AlertDialog.Builder(
				DeviceControlActivity.this);
		builder.setTitle("Error").setMessage(s).setCancelable(false)
				.setNegativeButton("ќ ", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						dialog.cancel();
					}
				});
		AlertDialog alert = builder.create();
		alert.show();
	}

	private void clearUI() {
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int progress,
			boolean fromUser) {
	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		if (mConnected) {
			if (seekBar == LightBar) {
				int t = (int) LightBar.getProgress();
				byte[] b = new byte[3];
				b[0] = 5;
				b[1] = (byte) (t >= 20 ? t - 20 : 0);
				b[2] = (byte) (b[1] + b[0]);
				icommand = 0;
				stack.push(b);
				makenextcommand();
				Log.e("LightBar", String.valueOf(LightBar.getProgress()));
			}
			if (seekBar == LedColor) {
				setled(LedColor.getProgress());
			}
		}
	}

	private void setled(int progress) {
		byte[] b = new byte[5];
		int cox = (colorbar.getWidth() * progress) / 1000;
		if (cox >= colorbar.getWidth())
			cox = colorbar.getWidth() - 1;
		int i = colorbar.getPixel(cox, 2);
		b[0] = 1;
		b[1] = (byte) (i >> 16);
		b[2] = (byte) (i >> 8);
		b[3] = (byte) (i);
		b[4] = (byte) (b[3] + b[1] + b[2] + b[0]);
		icommand = 0;
		stack.push(b);
		makenextcommand();
	}

	private void displayGattServices(List<BluetoothGattService> gattServices) {
		if (gattServices == null)
			return;
		String uuid = null;
		boolean dev = false;
		// Loops through available GATT Services.
		for (BluetoothGattService gattService : gattServices) {
			uuid = gattService.getUuid().toString();
			if (uuid.startsWith("0000fff0")) {
				dev = true;
				List<BluetoothGattCharacteristic> gattCharacteristics = gattService
						.getCharacteristics();
				for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
					uuid = gattCharacteristic.getUuid().toString();
					if (uuid.startsWith("0000fff4")) {
						myreadchar = gattCharacteristic;
					}
					if (uuid.startsWith("0000fff1")) {
						mywritechar = gattCharacteristic;
					}
				}
				if (myreadchar == null)
					Log.e("displayGattServices", "myreadchar not found");
				if (mywritechar == null)
					Log.e("displayGattServices", "mywritechar not found");

			}
		}
		if (!dev || myreadchar == null || mywritechar == null) {
			onBackPressed();
		} else
			startupread();
	}

	// -----------------------------вс€ка€ служебна€  хрень-----------------------------------
	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_WRITED);
		return intentFilter;
	}

}
