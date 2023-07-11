package com.app.artful;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.app.artful.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'artful' library on application startup.
    static {
        System.loadLibrary("artful");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //replaceMethod();
        try {
            swapBuildGetRadioVersion(MainActivity.class.getDeclaredMethod("maliciousMethod"));
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }

        // Setting screen text
        TextView tv = binding.sampleText;
        tv.setText(Build.getRadioVersion());
        //tv.setText(Build.getRadioVersion());
    }

    public native String replaceMethod();
    public native void swapBuildGetRadioVersion(Object targetObject);

    public static String benignMethod() {
        Log.d("ARTful", "Called benignMethod");
        return "I am benign";
    }

    public static String maliciousMethod() {
        Log.d("ARTful", "Called maliciousMethod");
        return "lol";
    }
}