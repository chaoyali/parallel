package fastandroid.neoncore.matrix.Util;

public class MatrixShort {
    private short[] data;
    private int M;
    private int N;

    public MatrixShort(short[] data, int M, int N) {
        this.data = data;
        this.M = M;
        this.N = N;
    }

    public MatrixShort() {

    }

    public short[] getData() {
        return data;
    }

    public int getM() {
        return M;
    }

    public int getN() {
        return N;
    }

    public void setData(short[] data) {
        this.data = data;
    }

    public void setM(int m) {
        M = m;
    }

    public void setN(int n) {
        N = n;
    }
}