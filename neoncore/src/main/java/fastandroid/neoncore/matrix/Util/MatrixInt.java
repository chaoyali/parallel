package fastandroid.neoncore.matrix.Util;

public class MatrixInt {
    private int[] data;
    private int M;
    private int N;

    public MatrixInt(int[] data, int M, int N) {
        this.data = data;
        this.M = M;
        this.N = N;
    }

    public MatrixInt() {

    }

    public int[] getData() {
        return data;
    }

    public int getM() {
        return M;
    }

    public int getN() {
        return N;
    }

    public void setData(int[] data) {
        this.data = data;
    }

    public void setM(int m) {
        M = m;
    }

    public void setN(int n) {
        N = n;
    }

    public void print() {
        int length = data.length;
        for (int i = 0; i < length; i++) {
            System.out.print(data[i] + " ");
        }
        System.out.println();
    }
}