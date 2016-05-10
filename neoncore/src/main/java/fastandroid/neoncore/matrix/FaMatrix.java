package fastandroid.neoncore.matrix;

import fastandroid.neoncore.matrix.Util.MatrixFloat;
import fastandroid.neoncore.matrix.Util.MatrixInt;
import fastandroid.neoncore.matrix.Util.MatrixShort;
import fastandroid.neoncore.matrix.Util.MatrixLong;

/**
 * Created by xgzhu on 5/5/16.
 */
public class FaMatrix {

    //addition
    private static native void matrix_int_add_intrin(int[] a, int m_a, int n_a, int[] b, int m_b, int n_b, int[] result);
    private static native void matrix_float_add_intrin(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);


    // multiply
    private static native void matrix_int_mul_sequential(int[] a, int m_a, int n_a, int[] b, int m_b, int n_b, int[] result);
    //private static native void matrix_int_mul_intrin(int[] a, int m_a, int n_a, int[] b, int m_b, int n_b, int[] result);
    private static native void matrix_int_mul_block_sequential(int[] a, int m_a, int n_a, int[] b, int m_b, int n_b, int[] result);
    private static native void matrix_int_mul_block_intrin1(int[] a, int m_a, int n_a, int[] b, int m_b, int n_b, int[] result);

    //private static native void matrix_float_mul_intrin(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);
    private static native void matrix_float_mul_sequential(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);
    private static native void matrix_float_mul_block_sequential(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);
    private static native void matrix_float_mul_block_intrin1(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);
    private static native void matrix_float_mul_block_intrin2(float[] a, int m_a, int n_a, float[] b, int m_b, int n_b, float[] result);


    private static native void matrix_short_mul_intrin(short[] a, int m_a, int n_a, short[] b, int m_b, int n_b, short[] result);
    private static native void matrix_short_mul_sequential(short[] a, int m_a, int n_a, short[] b, int m_b, int n_b, short[] result);

    // transpose
    private static native void matrix_int_transpose_intrin(int[] a, int m_a, int n_a, int[] result);
    private static native void matrix_int_transpose_sequential(int[] a, int m_a, int n_a, int[] result);

    private static native void matrix_float_transpose_intrin(float[] a, int m_a, int n_a, float[] result);
    private static native void matrix_float_transpose_sequential(float[] a, int m_a, int n_a, float[] result);


    // test speed
    public static void matrix_mul_int_test(MatrixInt a, MatrixInt b, MatrixInt result, MatrixInt result2) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        long seqTime = 0, neonTime = 0;
        int iterationTimes = 1;

        for (int i = 0; i < iterationTimes; i++) {

            long millis = System.currentTimeMillis();
            // convert to array
            matrix_int_mul_sequential(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
            result.setM(a.getM());
            result.setN(b.getN());

            long millis1 = System.currentTimeMillis();

            matrix_int_mul_block_intrin1(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result2.getData());
            result2.setM(a.getM());
            result2.setN(b.getN());

            long millis2 = System.currentTimeMillis();

            seqTime += millis1 - millis;
            neonTime += millis2 - millis1;
        }

        seqTime /= iterationTimes;
        neonTime /= iterationTimes;

        System.out.println("time1: " + seqTime);
        System.out.println("time2: " + neonTime);
        System.out.println("SEEPX: " + (float)seqTime/neonTime);
    }

    // test speed
    public static void matrix_mul_float_test(MatrixFloat a, MatrixFloat b, MatrixFloat result, MatrixFloat result2) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        long millis = System.currentTimeMillis();
        // convert to array
        matrix_float_mul_sequential(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
        result.setM(a.getM());
        result.setN(b.getN());

        long millis1 = System.currentTimeMillis();

        matrix_float_mul_block_intrin1(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result2.getData());
        result2.setM(a.getM());
        result2.setN(b.getN());

        long millis2 = System.currentTimeMillis();

        System.out.println("time1: " + (millis1 - millis));
        System.out.println("time2: " + (millis2 - millis1));
    }

    // test speed
    public static void matrix_mul_short_test(MatrixShort a, MatrixShort b, MatrixShort result, MatrixShort result2) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        long millis = System.currentTimeMillis();
        // convert to array
        matrix_short_mul_sequential(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
        result.setM(a.getM());
        result.setN(b.getN());

        long millis1 = System.currentTimeMillis();

        matrix_short_mul_intrin(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result2.getData());
        result2.setM(a.getM());
        result2.setN(b.getN());

        long millis2 = System.currentTimeMillis();

        System.out.println("time1: " + (millis1 - millis));
        System.out.println("time2: " + (millis2 - millis1));
    }


    /**
     * multiply int
     * @param a
     * @param b
     * @param result: a*b
     */
    public static void matrix_mul_int(MatrixInt a, MatrixInt b, MatrixInt result) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied.");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        matrix_int_mul_block_intrin1(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());

        result.setM(a.getM());
        result.setN(b.getN());
    }

    public static void matrix_mul_float(MatrixFloat a, MatrixFloat b, MatrixFloat result) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied.");
            return;
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
            return;
        }

        if (a.getData() == null || b.getData() == null) {
            System.err.println("Error. Access Null Matrix.");
            return;
        }

        if (result.getData() == null) {
            result.setData(new float[a.getM()*b.getN()]);
            return;
        }

        if (result.getData().length != result.getM()*result.getN()) {
            System.err.println("Error. Wrong Parameters");
            return;
        }

        matrix_float_mul_block_intrin1(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());

        result.setM(a.getM());
        result.setN(b.getN());
    }

    public static void matrix_mul_float_v2(MatrixFloat a, MatrixFloat b, MatrixFloat result) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied.");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        //MatrixFloat bt = new MatrixFloat();
        float[] bt = new float[b.getM() * b.getN()];
//        matrix_float_transpose_sequential(b.getData(), b.getM(), b.getN(), bt.getData());
//        bt.setM(a.getN());
//        bt.setN(a.getM());

        if (b.getM() <= 8 || b.getN() <= 8) {
            matrix_float_transpose_sequential(b.getData(), b.getM(), b.getN(), bt);
        }
        else {
            matrix_float_transpose_intrin(b.getData(), b.getM(), b.getN(), bt);
        }

//        bt.setM(a.getN());
//        bt.setN(a.getM());


        matrix_float_mul_block_intrin2(a.getData(), a.getM(), a.getN(), bt, b.getN(), b.getM(), result.getData());

        result.setM(a.getM());
        result.setN(b.getN());
    }

    public static void matrix_mul_float_seq(MatrixFloat a, MatrixFloat b, MatrixFloat result) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied.");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        matrix_float_mul_sequential(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());

        result.setM(a.getM());
        result.setN(b.getN());
    }

    public static void matrix_mul_short(MatrixShort a, MatrixShort b, MatrixShort result) {
        // check format
        if (a.getN() != b.getM()) {
            System.err.println("Error. These two MatrixInt cannot be multiplied.");
        }
        if (a.getM() < 1 || a.getN() < 1 || b.getM() < 1 || b.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        matrix_short_mul_intrin(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
        result.setM(a.getM());
        result.setN(a.getN());
    }

    /**
     * transpose int. if size < 8*8, use sequential
     * @param a
     * @param result
     */
    public static void matrix_transpose_int(MatrixInt a, MatrixInt result) {
        if (a.getM() < 1 || a.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        if (a.getM() < 8 || a.getN() < 8) {
            matrix_int_transpose_sequential(a.getData(), a.getM(), a.getN(), result.getData());
        }
        else {
            matrix_int_transpose_intrin(a.getData(), a.getM(), a.getN(), result.getData());
        }

        result.setM(a.getN());
        result.setN(a.getM());
    }

    // test speed
    public static void matrix_transpose_int_test(MatrixInt a, MatrixInt result, MatrixInt result2) {

        long millis = System.currentTimeMillis();
        matrix_int_transpose_sequential(a.getData(), a.getM(), a.getN(), result2.getData());

        result.setM(a.getN());
        result.setN(a.getM());

        long millis1 = System.currentTimeMillis();

        matrix_int_transpose_intrin(a.getData(), a.getM(), a.getN(), result.getData());

        result2.setM(a.getN());
        result2.setN(a.getM());

        long millis2 = System.currentTimeMillis();

        System.out.println("sequential: " + (millis1 - millis));
        System.out.println("neon      : " + (millis2 - millis1));
    }

    /**
     * transpose float. if size < 8*8, use sequential
     * @param a
     * @param result
     */
    public static void matrix_transpose_float(MatrixFloat a, MatrixFloat result) {
        if (a.getM() < 1 || a.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        if (a.getM() <= 8 || a.getN() <= 8) {
            matrix_float_transpose_sequential(a.getData(), a.getM(), a.getN(), result.getData());
        }
        else {
            matrix_float_transpose_intrin(a.getData(), a.getM(), a.getN(), result.getData());
        }

        result.setM(a.getN());
        result.setN(a.getM());
    }

    public static void matrix_transpose_float_seq(MatrixFloat a, MatrixFloat result) {
        if (a.getM() < 1 || a.getN() < 1) {
            System.err.println("Error. Size error.");
        }

        matrix_float_transpose_sequential(a.getData(), a.getM(), a.getN(), result.getData());

        result.setM(a.getN());
        result.setN(a.getM());
    }

    // test speed
    public static void matrix_transpose_float_test(MatrixFloat a, MatrixFloat result, MatrixFloat result2) {

        long millis = System.currentTimeMillis();
        matrix_float_transpose_sequential(a.getData(), a.getM(), a.getN(), result2.getData());

        result.setM(a.getN());
        result.setN(a.getM());

        long millis1 = System.currentTimeMillis();

        matrix_float_transpose_intrin(a.getData(), a.getM(), a.getN(), result.getData());

        result2.setM(a.getN());
        result2.setN(a.getM());

        long millis2 = System.currentTimeMillis();

        System.out.println("sequential: " + (millis1 - millis));
        System.out.println("neon      : " + (millis2 - millis1));
    }

    public static void matrix_add_int(MatrixInt a, MatrixInt b, MatrixInt result) {
        matrix_int_add_intrin(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
    }

    public static void matrix_add_float(MatrixFloat a, MatrixFloat b, MatrixFloat result) {
        matrix_float_add_intrin(a.getData(), a.getM(), a.getN(), b.getData(), b.getM(), b.getN(), result.getData());
    }



    static {
        System.loadLibrary("neoncore");
    }
}
