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

    // ARTful library methods
    public native void replaceAppMethodByObject(Object targetObject, Object newObject);
    public native void replaceAppMethodBySignature(String targetClassName, String targetMethodName, String newClassName,
                                                   String newMethodName, String methodSignature);
    public native void replaceGetRadioVersionByObject(Object newObject);
    public native void replaceGetRadioVersionBySignature(String newClassName, String newMethodName);
    public native void printArtMethodOffsets();
    public native void replaceLogEByObject(Object newObject);
    public native void replaceLogEBySignature(String newClassName, String newMethodName);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //replaceAppMethodBySignature("com/app/artful/MainActivity", "benignMethod", "com/app/artful/MainActivity", "newMethod", "()Ljava/lang/String;");

        try {
            replaceLogEByObject(MainActivity.class.getDeclaredMethod("newMethod", String.class, String.class));
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
        //Log.e("ARTful", "I'm benign");

        // Setting screen text to build version
        TextView tv = binding.sampleText;
        int retVal = Log.e("ARTful", "I'm benign");
        System.out.println("retval: " + retVal);

        //printArtMethodOffsets();
        tv.setText(Build.getRadioVersion());
    }

    public static String benignMethod() {
        Log.d("ARTful", "I am benign");
        return "I am benign";
    }

    public static int newMethod(String tag, String msg) {
        Log.d("ARTful", "I should not execute >:)");

        // Your code goes here!


        return 1;
    }
}