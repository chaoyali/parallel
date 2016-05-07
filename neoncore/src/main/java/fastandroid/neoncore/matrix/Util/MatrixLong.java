package fastandroid.neoncore.matrix.Util;

public class MatrixLong {
    private long[] data;
    private int M;
    private int N;

    public MatrixLong(long[] data, int M, int N) {
        this.data = data;
        this.M = M;
        this.N = N;
    }

    public MatrixLong() {

    }

    public long[] getData() {
        return data;
    }

    public int getM() {
        return M;
    }

    public int getN() {
        return N;
    }

    public void setData(long[] data) {
        this.data = data;
    }

    public void setM(int m) {
        M = m;
    }

    public void setN(int n) {
        N = n;
    }
}