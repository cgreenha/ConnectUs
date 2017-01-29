package com.led.led;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

import java.io.IOException;
import java.util.UUID;


public class ledControl extends ActionBarActivity {

    Button btnDis, btnOrange, btnRed, btnGreen;
    SeekBar brightness;
    //TextView signalStrength;// lumn
    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    BluetoothGatt bluetoothGatt = null;
    BluetoothDevice device = null;
    private boolean isBtConnected = false;
    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS); //receive the address of the bluetooth device

        //view of the ledControl
        setContentView(R.layout.activity_led_control);

        //call the widgtes
        btnGreen = (Button)findViewById(R.id.button2);
        btnOrange = (Button)findViewById(R.id.button3);
        btnRed = (Button)findViewById(R.id.button5);
        btnDis = (Button)findViewById(R.id.button4);
        //brightness = (SeekBar)findViewById(R.id.seekBar);
        //lumn = (TextView)findViewById(R.id.lumn);
        //signalStrength = (TextView)findViewById(R.id.textViewSignalStrength);

        new ConnectBT().execute(); //Call the class to connect

        //commands to be sent to bluetooth
        btnGreen.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                turnGreenLed();      //method to turn on
            }
        });

        btnOrange.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                turnOrangeLed();   //method to turn off
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

        /*brightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true)
                {
                    lumn.setText(String.valueOf(progress));
                    try
                    {
                        btSocket.getOutputStream().write(String.valueOf(progress).getBytes());
                    }
                    catch (IOException e)
                    {

                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });*/
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                btSocket.close(); //close connection
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish(); //return to the first layout

    }

    private void turnOrangeLed()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write("O".toString().getBytes());   // Send O

            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    private void turnGreenLed()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write("G".toString().getBytes()); // Send G
                //signalStrength.append(bluetoothGatt.readRemoteRssi());
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    private void turnRedLed()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write("R".toString().getBytes());   // Send R

            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
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
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(ledControl.this, "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                 myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                 BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                 btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                 BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                 btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }

}
