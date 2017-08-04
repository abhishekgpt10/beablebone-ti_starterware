package com.example.ti_adkdemo;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import android.R.integer;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.support.v4.app.NavUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;
import com.example.bbonedroid.R;

public class MainActivity extends Activity implements Runnable{

	CompoundButton SwitchLED1;
	CompoundButton SwitchLED2;
	CompoundButton SwitchLED3;
	CompoundButton SwitchLED4;
	CompoundButton SwitchWD;
	CompoundButton SwitchGPIO2;
	CompoundButton SwitchGPIO3;
	CompoundButton SwitchGPIO14;
	CompoundButton SwitchGPIO15;
	
	CompoundButton SwitchGPIO6;
	CompoundButton SwitchGPIO7;
	CompoundButton SwitchGPIO12;
	CompoundButton SwitchGPIO13;
	
	CheckBox cb_r2;
	CheckBox cb_r3;
	CheckBox cb_r6;
	CheckBox cb_r7;
	CheckBox cb_r12;
	CheckBox cb_r13;
	CheckBox cb_r14;
	CheckBox cb_r15;
	CheckBox cb_w2;
	CheckBox cb_w3;
	CheckBox cb_w6;
	CheckBox cb_w7;
	CheckBox cb_w12;
	CheckBox cb_w13;
	CheckBox cb_w14;
	CheckBox cb_w15;
	
	
	static String TAG="TIACCDemo";
	private static final String ACTION_USB_PERMISSION = "com.example.ti_adkdemo.action.USB_PERMISSION";
	
	private UsbManager mUsbManager;
	private PendingIntent mPermissionIntent;
	private boolean mPermissionRequestPending;
	private static final int MESSAGE_CLOCK = 1;
	TextView rtc_text,read_value;
	
	private static final int LED_ON= 0x1;
	private static final int LED_OFF= 0x0;
	
	private static final int WD_ON= 0x1;
	private static final int WD_OFF= 0x0;
	
	private static final int GPIO_ON= 0x0;
	private static final int GPIO_OFF= 0x1;

	private static final int LED1=0x10;
	private static final int LED2= 0x20;
	private static final int LED3= 0x30;
	private static final int LED4= 0x40;
	private static final int WD= 0x50;
	
	private static final int GPIO1_2= 0x80;
	private static final int GPIO1_3= 0x90;
	private static final int GPIO1_14= 0xE0;
	private static final int GPIO1_15= 0xF0;
	private static final int GPIO1_6= 0x60;
	private static final int GPIO1_7= 0x70;
	private static final int GPIO1_12= 0xC0;
	private static final int GPIO1_13= 0xD0;

	private static final byte CMD_LED_CONTROL = 0x01;
	private static final byte CMD_RTC_CONTROL = 0x02;
	private static final byte CMD_WD_CONTROL = 0x03;
	private static final byte CMD_GPIO_CONTROL_OUTPUT = 0x04;
	private static final byte CMD_GPIO_CONTROL_INPUT = 0x05;
	

	UsbAccessory mAccessory;
	ParcelFileDescriptor mFileDescriptor;
	FileInputStream mInputStream;
	FileOutputStream mOutputStream;
	
	RTCMsg r1= new RTCMsg();
	int read_pin;
	
	protected class RTCMsg {
		byte hour;
		byte min;
		byte sec;
		int read_val;

		public RTCMsg(byte hour, byte min, byte sec,byte read_val) {
			this.hour = hour;
			this.min = min;
			this.sec = sec;
			this.read_val=read_val;
		}

		public RTCMsg() {
			// TODO Auto-generated constructor stub
		}

		public int getHour() {
			return hour;
		}
		public int getMin() {
			return min;
		}
		public int getSec() {
			return sec;
		}
		public int getRead_val() {
			return read_val;
		}
	}
	
	private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG,"on Receive......");
			String action = intent.getAction();
			if (ACTION_USB_PERMISSION.equals(action)) {
				synchronized (this) {
					UsbAccessory accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
					if (intent.getBooleanExtra(
							UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
						openAccessory(accessory);
					} else {
						Log.d(TAG, "permission denied for accessory "
								+ accessory);
					}
					mPermissionRequestPending = false;
				}
			} else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
				Log.d(TAG,"on ACTION_USB_ACCESSORY_DETACHED......");
				UsbAccessory accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
				if (accessory != null && accessory.equals(mAccessory)) {
					closeAccessory();
					finish();
				}
			}
		}
	};
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState); // Always call the superclass first
    	Log.d(TAG,"on Create......");
    	
		mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
		Log.d(TAG,"on mUsbManager......");
		mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(
				ACTION_USB_PERMISSION), 0);
		IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
		filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		Log.d(TAG,"on intent......");
		registerReceiver(mUsbReceiver, filter);
		Log.d(TAG,"on register receiver......");
		
		//Added later
		if (savedInstanceState != null) {
            // Restore value of members from saved state
			//get maccessory if stored in on saveinstancestate
			//openAccessory(mAccessory);
        }
        
//		if (getLastNonConfigurationInstance() != null) {
//			mAccessory = (UsbAccessory) getLastNonConfigurationInstance();
//			openAccessory(mAccessory);
//		}
		
		Log.d(TAG,"on mAccesory......");
       
       
       
        getActionBar().setDisplayHomeAsUpEnabled(true);
        enableControls(false);

		if (mAccessory != null) {
			showControls();
			
		} else {
			hideControls();
		}
				
		super.onCreate(savedInstanceState);
    }

    public void setSwitchControls()
    {
    	Log.d(TAG,"setSwitchControl...");
		cb_r2= (CheckBox) findViewById(R.id.cbr2);
		cb_w2= (CheckBox) findViewById(R.id.cbw2);
		cb_r3= (CheckBox) findViewById(R.id.cbr3);
		cb_w3= (CheckBox) findViewById(R.id.cbw3);
		cb_r6= (CheckBox) findViewById(R.id.cbr6);
		cb_w6= (CheckBox) findViewById(R.id.cbw6);
		cb_w7= (CheckBox) findViewById(R.id.cbw7);
		cb_r7= (CheckBox) findViewById(R.id.cbr7);
		cb_r12= (CheckBox) findViewById(R.id.cbr12);
		cb_w12= (CheckBox) findViewById(R.id.cbw12);
		cb_r13= (CheckBox) findViewById(R.id.cbr13);
		cb_w13= (CheckBox) findViewById(R.id.cbw13);
		cb_r14= (CheckBox) findViewById(R.id.cbr14);
		cb_w14= (CheckBox) findViewById(R.id.cbw14);
		cb_r15= (CheckBox) findViewById(R.id.cbr15);
		cb_w15= (CheckBox) findViewById(R.id.cbw15);
        
		  
		
    	SwitchLED1 = (CompoundButton) findViewById(R.id.led1);
		SwitchLED1.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	                  // perform logic
	                  Log.d(TAG,"onCheckedChanged:Checked");
	                  sendCommand(CMD_LED_CONTROL, LED1 | LED_ON);
	              }
	              else
	              {
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_LED_CONTROL, LED1 | LED_OFF);
	              }
	          }

	      });

		
		SwitchLED2 = (CompoundButton) findViewById(R.id.led2);
		SwitchLED2.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	                  // perform logic
	                  Log.d(TAG,"onCheckedChanged:Checked");
	                  sendCommand(CMD_LED_CONTROL, LED2 | LED_ON);
	              }
	              else
	              {
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_LED_CONTROL, LED2 | LED_OFF);
	              }
	          }

	      });

		SwitchLED3 = (CompoundButton) findViewById(R.id.led3);
		SwitchLED3.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	                  // perform logic
	                  Log.d(TAG,"onCheckedChanged:Checked");
	                  sendCommand(CMD_LED_CONTROL, LED3 | LED_ON);
	              }
	              else
	              {
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_LED_CONTROL, LED3 | LED_OFF);
	              }
	          }

	      });

		SwitchLED4 = (CompoundButton) findViewById(R.id.led4);
		SwitchLED4.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	                  // perform logic
	                  Log.d(TAG,"onCheckedChanged:Checked");
	                  sendCommand(CMD_LED_CONTROL, LED4 | LED_ON);
	              }
	              else
	              {
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_LED_CONTROL, LED4 | LED_OFF);
	              }
	          }

	      });
//watchdog		
		SwitchWD = (CompoundButton) findViewById(R.id.wd);
		SwitchWD.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	                  // perform logic
	            	  Toast.makeText(MainActivity.this, "Toggle LED switches continously to prevent reset",Toast.LENGTH_SHORT).show();
	                  Log.d(TAG,"onCheckedChanged:Checked");
	                  sendCommand(CMD_WD_CONTROL, WD | WD_ON);
	              }
	              else
	              {
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_WD_CONTROL, WD | WD_OFF);
	              }
	          }

	      });
		
		
	//GPIO	
	   SwitchGPIO2 = (CompoundButton) findViewById(R.id.gpio_2);
	   SwitchGPIO2.setOnCheckedChangeListener(new OnCheckedChangeListener()
		    {
		          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		          {
		              if ( isChecked )
		              {
		            	Toast.makeText(MainActivity.this, "GPIO1 Pin2 Enable",Toast.LENGTH_SHORT).show();
		          		cb_w2.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_2 | GPIO_ON);
				                cb_r2.setChecked(false);
		        			}
		        		});
		          		
		          		cb_r2.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_2 | GPIO_ON);
				                cb_w2.setChecked(false);
				                read_pin= r1.getRead_val();
		        			}
		        		});
		                  
		              }
		              else
		              {
		            	  Toast.makeText(MainActivity.this, "GPIO1 Pin2 Disable",Toast.LENGTH_SHORT).show(); 
		                  Log.d(TAG,"onCheckedChanged:Unchecked");
		                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_2 | GPIO_OFF);
		              }
		          }

		      });
			SwitchGPIO3 = (CompoundButton) findViewById(R.id.gpio_3);
			SwitchGPIO3.setOnCheckedChangeListener(new OnCheckedChangeListener()
			    {
			          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
			          {
			              if ( isChecked )
			              {
			            	  Toast.makeText(MainActivity.this, "GPIO1 Pin3 Enable",Toast.LENGTH_SHORT).show();
				          		cb_w3.setOnClickListener(new View.OnClickListener() {
				        			
				        			@Override
				        			public void onClick(View v) {
				        				// TODO Auto-generated method stub
				        				Log.d(TAG,"onCheckedChanged:Checked");
						                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_3 | GPIO_ON);
						                cb_r3.setChecked(false);
				        			}
				        		});
				          		
				          		cb_r3.setOnClickListener(new View.OnClickListener() {
				        			
				        			@Override
				        			public void onClick(View v) {
				        				// TODO Auto-generated method stub
				        				Log.d(TAG,"onCheckedChanged:Checked");
						                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_3 | GPIO_ON);
						                cb_w3.setChecked(false);
						                read_pin= r1.getRead_val();
				        			}
				        		});
			              }
			              else
			              {
			            	  Toast.makeText(MainActivity.this, "GPIO1 Pin3 Disable",Toast.LENGTH_SHORT).show();
			                  Log.d(TAG,"onCheckedChanged:Unchecked");
			                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_3 | GPIO_OFF);

			              }
			          }

			      });	
		SwitchGPIO6 = (CompoundButton) findViewById(R.id.gpio_6);
		SwitchGPIO6.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	        	  Toast.makeText(MainActivity.this, "GPIO1 Pin6 Enable",Toast.LENGTH_SHORT).show();
	              if ( isChecked )
	              {
		          		cb_w6.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_6 | GPIO_ON);
				                cb_r6.setChecked(false);
		        			}
		        		});
		          		
		          		cb_r6.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_6 | GPIO_ON);
				                cb_w6.setChecked(false);
				                read_pin= r1.getRead_val();
		        			}
		        		});
	              }
	              else
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin6 Disable",Toast.LENGTH_SHORT).show();
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_6 | GPIO_OFF);
	              }
	          }

	      });
		
		
		SwitchGPIO7 = (CompoundButton) findViewById(R.id.gpio_7);
		SwitchGPIO7.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin7 Enable",Toast.LENGTH_SHORT).show();
		          		cb_w7.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_7 | GPIO_ON);
				                cb_r7.setChecked(false);
		        			}
		        		});
		          		
		          		cb_r7.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_7 | GPIO_ON);
				                cb_w7.setChecked(false);
				                read_pin= r1.getRead_val();
		        			}
		        		});
	              }
	              else  
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin7 Disable",Toast.LENGTH_SHORT).show();
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_7 | GPIO_OFF);
	              }
	          }

	      });
		
		SwitchGPIO12 = (CompoundButton) findViewById(R.id.gpio_12);
		SwitchGPIO12.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin12 Enable",Toast.LENGTH_SHORT).show();
		          		cb_w12.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_12 | GPIO_ON);
				                cb_r12.setChecked(false);
				                
		        			}
		        		});
		          		
		          		cb_r12.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_12 | GPIO_ON);
				                cb_w12.setChecked(false);
				                read_pin= r1.getRead_val();
		        			}
		        		});
	              }
	              else
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin12 Disable",Toast.LENGTH_SHORT).show();
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_12 | GPIO_OFF);

	              }
	          }

	      });
		
		SwitchGPIO13 = (CompoundButton) findViewById(R.id.gpio_13);
		SwitchGPIO13.setOnCheckedChangeListener(new OnCheckedChangeListener()
	    {
	          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
	          {
	              if ( isChecked )
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin13 Enable",Toast.LENGTH_SHORT).show();
		          		cb_w13.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_13 | GPIO_ON);
				                cb_r13.setChecked(false);

		        			}
		        		});
		          		
		          		cb_r13.setOnClickListener(new View.OnClickListener() {
		        			
		        			@Override
		        			public void onClick(View v) {
		        				// TODO Auto-generated method stub
		        				Log.d(TAG,"onCheckedChanged:Checked");
				                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_13 | GPIO_ON);
				                cb_w13.setChecked(false);
				                read_pin= r1.getRead_val();
		        			}
		        		});
	              }
	              else
	              {
	            	  Toast.makeText(MainActivity.this, "GPIO1 Pin13 Disable",Toast.LENGTH_SHORT).show();
	                  Log.d(TAG,"onCheckedChanged:Unchecked");
	                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_13 | GPIO_OFF);
	              }
	          }

	      });
		
		SwitchGPIO14 = (CompoundButton) findViewById(R.id.gpio_14);
		SwitchGPIO14.setOnCheckedChangeListener(new OnCheckedChangeListener()
		    {
		          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		          {
		              if ( isChecked )
		              {
		            	  Toast.makeText(MainActivity.this, "GPIO1 Pin14 Enable",Toast.LENGTH_SHORT).show();
			          		cb_w14.setOnClickListener(new View.OnClickListener() {
			        			
			        			@Override
			        			public void onClick(View v) {
			        				// TODO Auto-generated method stub
			        				Log.d(TAG,"onCheckedChanged:Checked");
					                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_14 | GPIO_ON);
					                cb_r14.setChecked(false);
			        			}
			        		});
			          		
			          		cb_r14.setOnClickListener(new View.OnClickListener() {
			        			
			        			@Override
			        			public void onClick(View v) {
			        				// TODO Auto-generated method stub
			        				Log.d(TAG,"onCheckedChanged:Checked");
					                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_14 | GPIO_ON);
					                cb_w14.setChecked(false);
					                read_pin= r1.getRead_val();
			        			}
			        		});
		              }
		              else
		              {
		            	  Toast.makeText(MainActivity.this, "GPIO1 Pin14 Disable",Toast.LENGTH_SHORT).show();
		                  Log.d(TAG,"onCheckedChanged:Unchecked");
		                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_14 | GPIO_OFF);
		              }
		          }

		      });
			
			SwitchGPIO15 = (CompoundButton) findViewById(R.id.gpio_15);
			SwitchGPIO15.setOnCheckedChangeListener(new OnCheckedChangeListener()
			    {
			          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
			          {
			              if ( isChecked )
			              {
			            	  Toast.makeText(MainActivity.this, "GPIO1 Pin15 Enable",Toast.LENGTH_SHORT).show();
				          		cb_w15.setOnClickListener(new View.OnClickListener() {
				        			
				        			@Override
				        			public void onClick(View v) {
				        				// TODO Auto-generated method stub
				        				Log.d(TAG,"onCheckedChanged:Checked");
						                sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_15 | GPIO_ON);
						                cb_r15.setChecked(false);
				        			}
				        		});
				          		
				          		cb_r15.setOnClickListener(new View.OnClickListener() {
				        			
				        			@Override
				        			public void onClick(View v) {
				        				// TODO Auto-generated method stub
				        				Log.d(TAG,"onCheckedChanged:Checked");
						                sendCommand(CMD_GPIO_CONTROL_INPUT, GPIO1_15 | GPIO_ON);
						                cb_w15.setChecked(false);
						                read_pin= r1.getRead_val();
				        			}
				        		});
			              }
			              else
			              {
			            	  Toast.makeText(MainActivity.this, "GPIO1 Pin15 Disable",Toast.LENGTH_SHORT).show();
			                  Log.d(TAG,"onCheckedChanged:Unchecked");
			                  sendCommand(CMD_GPIO_CONTROL_OUTPUT, GPIO1_15 | GPIO_OFF);
			              }
			          }
			          
			          

			      });
		
		
		
		


    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }

    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                NavUtils.navigateUpFromSameTask(this);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    	private void openAccessory(UsbAccessory accessory) {
		mFileDescriptor = mUsbManager.openAccessory(accessory);
		Log.d(TAG,"on mFileDescriptio......");
		if (mFileDescriptor != null) {
			mAccessory = accessory;
			Log.d(TAG,"on mAccesrory......");
			FileDescriptor fd = mFileDescriptor.getFileDescriptor();
			mInputStream = new FileInputStream(fd);
			mOutputStream = new FileOutputStream(fd);
			Thread thread = new Thread(null, this, "TI-ADK");
			Log.d(TAG,"on Thread start......");
			thread.start();
			Log.d(TAG, "accessory opened");
			enableControls(true);
		} else {
			Log.d(TAG, "accessory open fail");
		}
	}
	
	private void closeAccessory() {
		enableControls(false);

		try {
			if (mFileDescriptor != null) {
				mFileDescriptor.close();
			}
		} catch (IOException e) {
		} finally {
			mFileDescriptor = null;
			mAccessory = null;
		}
	}
	
	public void run() {
		int ret = 0;
		byte[] buffer = new byte[16384];
//		int i;

		while (ret >= 0) {
			try {
				ret = mInputStream.read(buffer);
			} catch (IOException e) {
				break;
			}

		
			switch (buffer[0]) {
			
				case CMD_RTC_CONTROL:
					//String Time = buffer[1]+":" + buffer[2] +":" + buffer[3];
				       
				Message m = Message.obtain(mHandler, MESSAGE_CLOCK);
				m.obj = new RTCMsg(buffer[1], buffer[2], buffer[3],buffer[4]);
				mHandler.sendMessage(m);
					
					break;
					default:
						Log.d(TAG, "unknown msg: " + buffer[0]);
					break;
			}
		}
		
	}

	
	Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MESSAGE_CLOCK:
				RTCMsg o = (RTCMsg) msg.obj;
				String Time = o.hour +":"+ o.min + ":" + o.sec;
				String value=  Integer.toString(o.read_val);
				try {
					rtc_text = (TextView) findViewById(R.id.rtc_text); 
					rtc_text.setText(Time);
					read_value = (TextView) findViewById(R.id.value);
					read_value.setText(value);
				}
				catch(Exception io)
				{
					Log.d(TAG,"Do Nothing.!!!!");
				};
				break;
			}
		}
	};
	public void sendCommand(byte command,int value) {
		byte[] buffer = new byte[5];
		if (value > 255)
			value = 255;

		buffer[0] = command;
		buffer[1] = (byte) (value & 0xFF);
		buffer[2] = (byte) ((value >> 8 ) & 0xFF);
		buffer[3] = (byte) ((value >> 16 ) & 0xFF);
		buffer[4] = (byte) ((value >> 24 ) & 0xFF);
		
		if (mOutputStream != null && buffer[1] != -1) {
			try {
				mOutputStream.write(buffer);
			} catch (IOException e) {
				Log.e(TAG, "write failed.", e);
			}
		}
	}
       
	@Override
	public void onResume() {
		super.onResume();

//		Intent intent = getIntent();
		if (mInputStream != null && mOutputStream != null) {
			return;
		}

		UsbAccessory[] accessories = mUsbManager.getAccessoryList();
		UsbAccessory accessory = (accessories == null ? null : accessories[0]);
		if (accessory != null) {
			if (mUsbManager.hasPermission(accessory)) {
				openAccessory(accessory);
			} else {
				synchronized (mUsbReceiver) {
					if (!mPermissionRequestPending) {
						mUsbManager.requestPermission(accessory,
								mPermissionIntent);
						mPermissionRequestPending = true;
					}
				}
			}
		} else {
			Log.d(TAG, "mAccessory is null");
		}
	}

	@Override
	public void onPause() {
		super.onPause();
		closeAccessory();
		Log.d(TAG,"on Pause ");
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(mUsbReceiver);
		Log.d(TAG,"on Destroy");
		super.onDestroy();
	}
	
	//added for new API support
	@Override
	protected void onSaveInstanceState(Bundle savedInstanceState) {
		//save maccessory in bundle
		super.onSaveInstanceState(savedInstanceState);

	}
	
	@Override
	protected void onRestoreInstanceState (Bundle savedInstanceState) {
		 super.onRestoreInstanceState(savedInstanceState);
		 //get m accessory from bundle
	}
	
	protected void enableControls(boolean enable) {
		if (enable) {
			showControls();
		} else {
			hideControls();
		}
	}

	protected void hideControls() {
		setContentView(R.layout.no_device);
	
	}

	protected void showControls() {
		setContentView(R.layout.activity_main);
		setSwitchControls();
		
	}

}