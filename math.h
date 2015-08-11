#ifndef PLAYGROUND_MATH_H
#define PLAYGROUND_MATH_H

namespace math {
    template <typename T>
    class Matrix {
    private:
        int ROW_DIM, COL_DIM, PROD;
        T *matrix;
    public:
        Matrix(int row_dim, int col_dim, const T *arr): ROW_DIM(row_dim), COL_DIM(col_dim) {
            PROD = ROW_DIM * COL_DIM;
            matrix = new T[PROD];
            int source_current = 0;
            for (int c = 0; c < COL_DIM; ++c) {
                for (int r = 0; r < ROW_DIM; ++r) {
                    int offset = r * COL_DIM + c;
                    matrix[offset] = arr[source_current++];
                }
            }
        }

        inline T get(int row_index, int col_index) const {
            return matrix[row_index * COL_DIM + col_index];
        }

        const T operator [] (int offset) const {
            return matrix[offset];
        }

        ~Matrix() {
            if (matrix) {
                delete[] matrix;
            }
        }
    };
}

#endif //PLAYGROUND_MATH_H
