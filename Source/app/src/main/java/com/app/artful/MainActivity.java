package com.app.artful;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import java.util.regex.Pattern;

import com.app.artful.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'artful' library on application startup.
    static {
        System.loadLibrary("artful");
    }

    private ActivityMainBinding binding;

    // ARTful library methods
    public native void replaceAppMethodByObject(Object targetObject, Object newObject);
    public native void replaceAppMethodBySignature(String targetClassName, String targetMethodName, String newClassName,
                                                   String newMethodName, String methodSignature);
    public native void printArtMethodOffsets();
    public native void replaceGetRadioVersionByObject(Object newObject);
    public native void replaceGetRadioVersionBySignature(String newClassName, String newMethodName);
    public native void replaceLogEByObject(Object newObject);
    public native void replaceLogEBySignature(String newClassName, String newMethodName);
    public native void replaceToastMakeTextByObject(Object newObject);
    public native void replaceToastMakeTextBySignature(String newClassName, String newMethodName);
    public native void replacePatternMatchesByObject(Object newObject);
    public native void replacePatternMatchesBySignature(String newClassName, String newMethodName);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        try {
            replaceGetRadioVersionByObject(MainActivity.class.getDeclaredMethod("newMethod"));
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }

        TextView tv = binding.sampleText;
        tv.setText(Build.getRadioVersion());
    }

    public static String benignMethod() {
        Log.d("ARTful", "I am benign");
        return "I am benign";
    }

    public static String newMethod() {
        Log.d("ARTful", "I should not execute >:)");

        // Your code goes here!

        return "lol";
    }
}