package fastandroid.neoncore.collection;

/**
 * Created by xgzhu on 5/5/16.
 */
public class FaCollection {

    // TEST FUNCTION

    public static String test_vector() {
        return vector_int_test();
    }

    public static String test_sort() {
        return sort_int_test();
    }

    public static String test_fft()
    {
        return fft_float_test();
    }



    // Java interface
    public static void vector(Object[] array, Object[] vars) {
        if (array[0] instanceof Integer && vars[0] instanceof Integer) {
            int[] ia = new int[array.length], ir = new int[vars.length];
            for (int i = 0; i < array.length; i++) {
                ia[i] = ((Integer)array[i]).intValue();
                ir[i] = ((Integer)vars[i]).intValue();
            }
            vector_int(ia, array.length, ir, ir.length);
            for (int i = 0; i < array.length; i++) {
                array[i] = ia[i];
            }
        }
        else if (array[0] instanceof Float && vars[0] instanceof Float) {
            float[] ia = new float[array.length], ir = new float[vars.length];
            for (int i = 0; i < array.length; i++) {
                ia[i] = ((Float)array[i]).floatValue();
                ir[i] = ((Float)vars[i]).floatValue();
            }
            vector_float(ia, array.length, ir, ir.length);
            for (int i = 0; i < array.length; i++) {
                array[i] = ia[i];
            }
        }
        else {
            System.err.println("Error. We current do not support the input type in vector.");
        }
    }

    public static void vector_int32(int[] array, int[] vars) {
        vector_int(array, array.length, vars, vars.length);
    }

    public static void vector_float32(float[] array, float[] vars) {
        vector_float(array, array.length, vars, vars.length);
    }

//    public static void vector_comp_float32(float[] real, float[] imag, float[] vars) {
//        if (real.length != imag.length) return; //error!
//        vector_complex(real, imag, real.length, vars, vars.length);
//    }

    public static void qsort(Object[] array) {
        if (array[0] instanceof Integer) {
            int[] ia = new int[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Integer)array[i]).intValue();
            qsort_int(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else if (array[0] instanceof Float) {
            float[] ia = new float[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Float)array[i]).floatValue();
            qsort_float(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else if (array[0] instanceof Double) {
            double[] ia = new double[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Double)array[i]).doubleValue();
            qsort_double(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else {
            System.err.println("Error. We current do not support the input type in qsort.");
        }
    }

    public  static void qsort_int32(int[] array) {
        qsort_int(array, array.length);
    }

    public  static void qsort_float32(float[] array) {
        qsort_float(array, array.length);
    }

    public  static void qsort_double32(double[] array) {
        qsort_double(array, array.length);
    }


    public static void sort(Object[] array) {
        if (array[0] instanceof Integer) {
            int[] ia = new int[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Integer)array[i]).intValue();
            sort_int(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else if (array[0] instanceof Float) {
            float[] ia = new float[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Float)array[i]).floatValue();
            sort_float(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else if (array[0] instanceof Double) {
            double[] ia = new double[array.length];
            for (int i = 0; i < array.length; i++) ia[i] = ((Double)array[i]).doubleValue();
            qsort_double(ia, array.length);
            for (int i = 0; i < array.length; i++) array[i] = ia[i];
        }
        else {
            System.err.println("Error. We current do not support the input type in sort.");
        }
    }

    public  static void sort_float32(float[] array) {
        sort_float(array, array.length);
    }

    public  static void sort_int32(int[] array) {
        sort_int(array, array.length);
    }

//    public  static void int_sort_c(int[] array) {
//        sort_int_c(array, array.length);
//    }

    public static void fft(Object[] real, Object[] imag) {
        if (real.length != imag.length) return; // throw error

        int len = real.length;
        int m = (int) (Math.log(len) / Math.log(2));
        if (len != 1<<m) return;

        if (real[0] instanceof Float) {
            float[] ar = new float[real.length];
            float[] ai = new float[imag.length];
            for (int i = 0; i < len; i ++) {
                ar[i] = ((Float)real[i]).floatValue();
                ai[i] = ((Float)imag[i]).floatValue();
            }
            fft_float(ar, ai, len, 0);
            for (int i = 0; i < len; i ++) {
                real[i] = ar[i];
                imag[i] = ai[i];
            }
        }
        else {
            System.err.println("Error. We current do not support the input type in fft.");
        }
    }

    public static void fft_float32(float[] real, float[] imag) {
        if (real.length != imag.length) {
            return;
        }

        int len = real.length;
        int m = (int) (Math.log(len) / Math.log(2));
        if (len != 1<<m) return;

        fft_float(real, imag, len, 0);
    }

    public static void ifft(Object[] real, Object[] imag) {
        if (real.length != imag.length) return; // throw error

        int len = real.length;
        int m = (int) (Math.log(len) / Math.log(2));
        if (len != 1<<m) return;

        if (real[0] instanceof Float) {
            float[] ar = new float[real.length];
            float[] ai = new float[imag.length];
            for (int i = 0; i < len; i ++) {
                ar[i] = ((Float)real[i]).floatValue();
                ai[i] = ((Float)imag[i]).floatValue();
            }
            fft_float(ar, ai, len, 1);
            for (int i = 0; i < len; i ++) {
                real[i] = ar[i];
                imag[i] = ai[i];
            }
        }
        else {
            System.err.println("Error. We current do not support the input type in ifft.");
        }
    }

    public static void ifft_float32(float[] real, float[] imag) {
        if (real.length != imag.length) return; // throw error

        int len = real.length;
        int m = (int) (Math.log(len) / Math.log(2));
        if (len != 1<<m) return;

        fft_float(real, imag, len, 1);
    }


    // For testing
    private static native String fft_float_test();
    private static native String sort_int_test();
    private static native String vector_int_test();
    private static native void sort_int_c(int[] array, int len);
    private static native void fft_float_c(float[] real, float[] imag, int len, int reverse);

    // For calling
    private static native void vector_int(int[]x, int len, int[]var, int num_var);
    private static native void vector_float(float[]x, int len, float[]var, int num_var);
    private static native void fft_float(float[] real, float[] imag, int len, int reverse);
    private static native void sort_int(int[] array, int len);
    private static native void sort_float(float[] array, int len);
    private static native void qsort_int(int[] array, int len);
    private static native void qsort_float(float[] array, int len);
    private static native void qsort_double(double[] array, int len);

    static {
        System.loadLibrary("neoncore");
    }
}
