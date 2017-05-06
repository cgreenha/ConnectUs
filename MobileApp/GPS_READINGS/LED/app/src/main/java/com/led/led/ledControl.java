package com.led.led;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.location.Location;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;

import java.io.IOException;
import java.util.UUID;

import static com.led.led.R.id.buttong;


public class ledControl extends ActionBarActivity{
    Button btnDis, btnOrange, btnRed, btnGreen, btnAlert, btnNotif, btnLocate, btnAltO;
    RadioButton btn15, btn110, btn1U, btn210, btn220, btn2U;
    SeekBar brightness;
    EditText textView1,textView2;
    TextView textView3, textView4, textView5;
    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    BluetoothGatt bluetoothGatt = null;
    BluetoothDevice device = null;
    private boolean isBtConnected = false;
    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private GoogleApiClient googleApiClient;
    private static final int PERMISSION_ACCESS_FINE_LOCATION = 1;
    private byte[] mmBuffer;
    private int mInterval = 1000; // 5 seconds by default, can be changed later
    private Handler mHandler;
    private int range1 = 5;
    private int range2 = 10;
    private String str2 ="";
    private double clon;
    private double clat;
    private double cwlat = 0;
    private double cwlon = 0;
    private int ii = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
new CountDownTimer(10800000, 1800000){
    public void onTick(long millisUntilFinished){
        double percent = millisUntilFinished/108000;
        notifyc(percent);
    }
    public void onFinish(){
        notifyb();
    }
}.start();


        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[] { Manifest.permission.ACCESS_FINE_LOCATION },
                    PERMISSION_ACCESS_FINE_LOCATION);
        }

        //googleApiClient = new GoogleApiClient.Builder(this).addApi(LocationServices.API).build();
        // Create an instance of GoogleAPIClient.
        if (googleApiClient == null) {
            googleApiClient = new GoogleApiClient.Builder(this)
                    //.addConnectionCallbacks(this)
                    //.addOnConnectionFailedListener(this)
                    .addApi(LocationServices.API)
                    .build();
        }
        createLocationRequest();


        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS); //receive the address of the bluetooth device

        //view of the ledControl
        setContentView(R.layout.activity_led_control);

        //call the widgtes
        btnGreen = (Button)findViewById(buttong);
        btnOrange = (Button)findViewById(R.id.buttono);
        btnRed = (Button)findViewById(R.id.buttonr);
        btnDis = (Button)findViewById(R.id.buttond);
        btnAlert = (Button)findViewById(R.id.buttona);
        btnNotif = (Button)findViewById(R.id.button2);
        btnLocate = (Button)findViewById(R.id.buttonlocate);
        btnAltO = (Button)findViewById(R.id.btnAO);
        btn1U = (RadioButton)findViewById(R.id.radioButton1U);
        btn2U = (RadioButton)findViewById(R.id.radioButton2U);
        btn15 = (RadioButton)findViewById(R.id.radioButton15);
        btn110 = (RadioButton)findViewById(R.id.radioButton110);
        btn210 = (RadioButton)findViewById(R.id.radioButton210);
        btn220 = (RadioButton)findViewById(R.id.radioButton220);
        textView1 = (EditText) findViewById(R.id.textView1);
        //textView2 = (EditText) findViewById(R.id.textView2);
        //textView3 = (TextView) findViewById(R.id.textView3);
        //textView4 = (TextView) findViewById(R.id.textView4);
        //textView5 = (TextView) findViewById(R.id.textView5);


        new ConnectBT().execute(); //Call the class to connect

        //commands to be sent to bluetooth
        btn110.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                range1(20);
            }
        });

        btn15.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                range1(10);
            }
        });

        btn210.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                range2(20);
            }
        });

        btn220.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                range2(40);
            }
        });

        btn1U.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {range1(Integer.valueOf(textView1.getText().toString()));}
        });

        btn2U.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {range2(Integer.valueOf(textView2.getText().toString()));}
        });

        btnNotif.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                notifyd("TESTING");
            }
        });

        btnAltO.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                alertOff();
            }
        });

        btnGreen.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                turnGreenLed();
            }
        });

        btnOrange.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                turnOrangeLed();
            }
        });

        btnAlert.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                alert();
            }
        });

        btnRed.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                turnRedLed();   //method to turn off
            }
        });

        btnDis.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Disconnect(); //close connection
            }
        });

        btnLocate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                startRepeatingTask();
            }
        });

        mHandler = new Handler();



    }




    //Location Area

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_ACCESS_FINE_LOCATION:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // All good!
                } else {
                    Toast.makeText(this, "Need your location!", Toast.LENGTH_SHORT).show();
                }

                break;
        }
    }

    @Override
    protected void onStart() {
        if (googleApiClient != null) {
            googleApiClient.connect();
        }
        super.onStart();

    }

    @Override
    protected void onStop() {
        googleApiClient.disconnect();
        super.onStop();
    }

    protected void createLocationRequest() {
        LocationRequest mLocationRequest = new LocationRequest();
        mLocationRequest.setInterval(10000);
        mLocationRequest.setFastestInterval(5000);
        mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);

        //LocationSettingsRequest.Builder builder = new LocationSettingsRequest.Builder()
        //        .addLocationRequest(mLocationRequest);

        //PendingResult<LocationSettingsResult> result =
                //LocationServices.SettingsApi.checkLocationSettings(googleApiClient,
                        //builder.build());


    }

    public void locate(double lat,double lon) {
        Log.i(ledControl.class.getSimpleName(), "Connected to Google Play Services!");
        float[] f1 = new float[1];
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION)==
                PackageManager.PERMISSION_GRANTED) {
            Location lastLocation = LocationServices.FusedLocationApi.getLastLocation(googleApiClient);

            //textView3.setText(Double.toString(lon));
            //textView5.setText(Double.toString(lat));

            if(ii==0) {
                cwlat = lat;
                cwlon = lon;
                ii=1;
            }
            else {
                lastLocation.distanceBetween(lat, lon, cwlat, cwlon, f1);
                double dd = f1[0] * 3.28;
                textView1.setText(Double.toString(dd));

                if (f1[0] * 3.28 > range2) {
                    dangerZone();
                } else if (f1[0] * 3.28 > range1) {
                    warningZone();
                } else {
                    safeZone();
                }
            }

            /*Location lastLocation = LocationServices.FusedLocationApi.getLastLocation(googleApiClient);
            double lat = lastLocation.getLatitude(), lon = lastLocation.getLongitude();
            //textView3.setText(Double.toString(lon));
            //textView5.setText(Double.toString(lat));

            //if(ii==0) {
            // handilat = lat - cwlat;
            //handilon = lon - cwlon;
            //ii=1;
            //}

            lastLocation.distanceBetween(lat,lon,cwlat,cwlon,f1);
            double dd = f1[0]*3.28;
            textView1.setText(Double.toString(dd));

            if(f1[0]*3.28 > range2){
                dangerZone();
            }
            else if(f1[0]*3.28 > range1){
                warningZone();
            }
            else{
                safeZone();
            }*/
        }
    }

    void startRepeatingTask() {
        mStatusChecker.run();
    }
    Runnable mStatusChecker = new Runnable() {
        @Override
        public void run() {
            mmBuffer = new byte[150];
            int numBytes = 0; // bytes returned from read()
            int bytes = 0;
            String lgstr = "";

            {
                if (btSocket != null) {
                    try {
                        btSocket.getInputStream().read(mmBuffer); //, bytes, mmBuffer.length - bytes
                        // Send the obtained bytes to the UI activity.
                        String str1 = new String(mmBuffer);

                        str2 = str2 + str1;
                        //textView1.setText(str2);
                        //char[] charArray = str1.toCharArray();

                        String[] parts = str2.split("[,]");

                        for (int x = 1; x < parts.length; x++) {
                            if (parts[x].equals("N")) {
                                if (isDouble(parts[x - 1])) {
                                    clat = Double.parseDouble(parts[x - 1]) / 100;
                                    clat = (clat - Math.floor(clat)) / 60 * 100 + Math.floor(clat);
                                    //textView4.setText(Double.toString(clat));
                                }
                            }
                            if (parts[x].equals("W")) {
                                if (isDouble(parts[x - 1])) {
                                    clon = Double.parseDouble(parts[x - 1]) / 100;
                                    clon = -((clon - Math.floor(clon)) / 60 * 100 + Math.floor(clon));
                                    //textView2.setText(Double.toString(clon));
                                    str2 = "";
                                    locate(clat, clon);
                                }
                            }

                        }


                        //textView1.setText(Double.toString(value1));
                        //textView2.setText(Double.toString(value2));


                    } catch (IOException e) {
                        //msg("Error");
                    }
                }
            }
            mHandler.postDelayed(mStatusChecker, mInterval);
        }
    };


    public static boolean isDouble(String s) {
        try {
            Double.parseDouble(s);
        } catch(NumberFormatException e) {
            return false;
        } catch(NullPointerException e) {
            return false;
        }
        // only got here if we didn't return false
        return true;
    }
        public void onConnectionFailedlistner() {
            Log.i(ledControl.class.getSimpleName(), "Can't connect to Google Play Services!");
        }


        //Push Notification
    public void notifyd(String zone) {

        NotificationManager notif = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notify = new Notification.Builder
                (getApplicationContext()).setContentTitle("Warning").setContentText("Child Device Has Entered The " + zone + " Zone").
                setContentTitle("Warning").setSmallIcon(R.drawable.abc_btn_radio_material).build();
        notify.flags = Notification.FLAG_SHOW_LIGHTS | Notification.FLAG_AUTO_CANCEL;
        //notify.ledARGB = color.BLUE;//
        notify.ledOnMS = 1000;
        notify.ledOffMS = 300;
        notify.defaults |= Notification.DEFAULT_SOUND;
        notify.defaults |= Notification.DEFAULT_VIBRATE;
        notif.notify(0, notify);
    }
    public void notifyb() {

        NotificationManager notif = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notify = new Notification.Builder
                (getApplicationContext()).setContentTitle("Warning").setContentText("Please Recharge The Child Device").
                setContentTitle("Warning").setSmallIcon(R.drawable.abc_btn_radio_material).build();
        notify.flags = Notification.FLAG_SHOW_LIGHTS | Notification.FLAG_AUTO_CANCEL;
        //notify.ledARGB = color.BLUE;//
        notify.ledOnMS = 1000;
        notify.ledOffMS = 300;
        notify.defaults |= Notification.DEFAULT_SOUND;
        notify.defaults |= Notification.DEFAULT_VIBRATE;
        notif.notify(0, notify);
    }
    public void notifyc(double percent) {

        NotificationManager notif = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notify = new Notification.Builder
                (getApplicationContext()).setContentTitle("Warning").setContentText("Child Device At " + percent + "% battery remianing").
                setContentTitle("Warning").setSmallIcon(R.drawable.abc_btn_radio_material).build();
        notify.flags = Notification.FLAG_SHOW_LIGHTS | Notification.FLAG_AUTO_CANCEL;
        //notify.ledARGB = color.BLUE;//
        notify.ledOnMS = 1000;
        notify.ledOffMS = 300;
        notify.defaults |= Notification.DEFAULT_SOUND;
        notify.defaults |= Notification.DEFAULT_VIBRATE;
        notif.notify(0, notify);
    }



    private void Disconnect() {
            if (btSocket != null) //If the btSocket is busy
            {
                try {
                    btSocket.getOutputStream().write("~".toString().getBytes());
                    btSocket.close(); //close connection
                } catch (IOException e) {
                    msg("Error");
                }
            }
            finish(); //return to the first layout

        }

        private void alert() {
            if (btSocket != null) {
                try {
                    btSocket.getOutputStream().write("@".toString().getBytes());   // Send O

                } catch (IOException e) {
                    msg("Error");
                }
            }
        }

        private void turnOrangeLed() {
            if (btSocket != null) {
                try {
                    btSocket.getOutputStream().write("X".toString().getBytes());   // Send O

                } catch (IOException e) {
                    msg("Error");
                }
            }
        }

        private void turnGreenLed() {
            if (btSocket != null) {
                try {
                    btSocket.getOutputStream().write("Y".toString().getBytes()); // Send G
                    //signalStrength.append(bluetoothGatt.readRemoteRssi());
                } catch (IOException e) {
                    msg("Error");
                }
            }
        }

        private void range1(int x) {
            range1 = x;
        }

        private void range2(int x) {
            range2 = x;
        }

        private void turnRedLed() {
            if (btSocket != null) {
                try {
                    btSocket.getOutputStream().write("Z".toString().getBytes());   // Send R

                } catch (IOException e) {
                    msg("Error");
                }
            }
        }

    private void alertOff() {
        if (btSocket != null) {
            try {
                btSocket.getOutputStream().write("&".toString().getBytes());   // Send R

            } catch (IOException e) {
                msg("Error");
            }
        }
    }

        //Final Distance Functions

        private void dangerZone() {
            turnRedLed();
            notifyd("Danger");
            btnGreen.setVisibility(View.GONE);
            btnOrange.setVisibility(View.GONE);
            btnRed.setVisibility(View.VISIBLE);
        }

        private void warningZone() {
            turnOrangeLed();
            notifyd("Warning");
            btnGreen.setVisibility(View.GONE);
            btnOrange.setVisibility(View.VISIBLE);
            btnRed.setVisibility(View.GONE);
        }

        private void safeZone() {
            turnGreenLed();
            btnGreen.setVisibility(View.VISIBLE);
            btnOrange.setVisibility(View.GONE);
            btnRed.setVisibility(View.GONE);
        }


        // fast way to call Toast
        private void msg(String s) {
            Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG).show();
        }

        @Override
        public boolean onCreateOptionsMenu(Menu menu) {
            // Inflate the menu; this adds items to the action bar if it is present.
            getMenuInflater().inflate(R.menu.menu_led_control, menu);
            return true;
        }

        @Override
        public boolean onOptionsItemSelected(MenuItem item) {
            // Handle action bar item clicks here. The action bar will
            // automatically handle clicks on the Home/Up button, so long
            // as you specify a parent activity in AndroidManifest.xml.
            int id = item.getItemId();

            //noinspection SimplifiableIfStatement
            if (id == R.id.action_settings) {
                return true;
            }

            return super.onOptionsItemSelected(item);
        }

        private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
        {
            private boolean ConnectSuccess = true; //if it's here, it's almost connected

            @Override
            protected void onPreExecute() {
                progress = ProgressDialog.show(ledControl.this, "Connecting...", "Please wait!!!");  //show a progress dialog
            }

            @Override
            protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
            {
                try {
                    if (btSocket == null || !isBtConnected) {
                        myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                        BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                        btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                        btSocket.connect();//start connection
                    }
                } catch (IOException e) {
                    ConnectSuccess = false;//if the try failed, you can check the exception here
                }

                return null;
            }

            @Override
            protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
            {
                super.onPostExecute(result);

                if (!ConnectSuccess) {
                    msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                    finish();
                } else {
                    msg("Connected.");
                    isBtConnected = true;
                    try {
                        btSocket.getOutputStream().write("^".toString().getBytes());    // send B to let microcontroller know it is connected
                    }
                    catch(IOException e){
                        ConnectSuccess = false;//if the try failed, you can check the exception here
                    }
                }
                progress.dismiss();
            }
        }

    }
