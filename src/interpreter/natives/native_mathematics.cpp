#include "native_mathematics.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace claw {

// ============================================================
// Advanced Mathematics Implementation
// ============================================================
struct MathEngine {
    // Matrix operations
    struct Matrix {
        std::vector<std::vector<double>> data;
        int rows, cols;
        
        Matrix(int r, int c) : rows(r), cols(c) {
            data.resize(rows, std::vector<double>(cols, 0.0));
        }
        
        Matrix(int r, int c, const std::vector<double>& values) : rows(r), cols(c) {
            data.resize(rows, std::vector<double>(cols, 0.0));
            for (int i = 0; i < rows && i * cols < values.size(); ++i) {
                for (int j = 0; j < cols && i * cols + j < values.size(); ++j) {
                    data[i][j] = values[i * cols + j];
                }
            }
        }
        
        Matrix multiply(const Matrix& other) const {
            if (cols != other.rows) {
                return Matrix(0, 0); // Invalid multiplication
            }
            
            Matrix result(rows, other.cols);
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < other.cols; ++j) {
                    for (int k = 0; k < cols; ++k) {
                        result.data[i][j] += data[i][k] * other.data[k][j];
                    }
                }
            }
            return result;
        }
        
        Matrix transpose() const {
            Matrix result(cols, rows);
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    result.data[j][i] = data[i][j];
                }
            }
            return result;
        }
        
        double determinant() const {
            if (rows != cols) return 0.0;
            
            if (rows == 1) return data[0][0];
            if (rows == 2) return data[0][0] * data[1][1] - data[0][1] * data[1][0];
            
            // Laplace expansion for larger matrices
            double det = 0.0;
            for (int j = 0; j < cols; ++j) {
                Matrix sub(rows - 1, cols - 1);
                for (int i = 1; i < rows; ++i) {
                    for (int k = 0, col = 0; k < cols; ++k) {
                        if (k != j) {
                            sub.data[i - 1][col++] = data[i][k];
                        }
                    }
                }
                det += (j % 2 == 0 ? 1 : -1) * data[0][j] * sub.determinant();
            }
            return det;
        }
        
        Matrix inverse() const {
            if (rows != cols) return Matrix(0, 0);
            
            double det = determinant();
            if (fabs(det) < 1e-10) return Matrix(0, 0); // Singular matrix
            
            Matrix result(rows, cols);
            // Simplified inverse for 2x2 matrix
            if (rows == 2) {
                double invDet = 1.0 / det;
                result.data[0][0] = data[1][1] * invDet;
                result.data[0][1] = -data[0][1] * invDet;
                result.data[1][0] = -data[1][0] * invDet;
                result.data[1][1] = data[0][0] * invDet;
            }
            
            return result;
        }
    };
    
    // Vector operations
    struct Vector {
        std::vector<double> components;
        
        Vector(int size) : components(size, 0.0) {}
        Vector(const std::vector<double>& comps) : components(comps) {}
        
        double magnitude() const {
            double sum = 0.0;
            for (double comp : components) {
                sum += comp * comp;
            }
            return sqrt(sum);
        }
        
        Vector normalize() const {
            double mag = magnitude();
            if (mag == 0.0) return Vector(components.size());
            
            Vector result(components.size());
            for (size_t i = 0; i < components.size(); ++i) {
                result.components[i] = components[i] / mag;
            }
            return result;
        }
        
        double dot(const Vector& other) const {
            if (components.size() != other.components.size()) return 0.0;
            
            double result = 0.0;
            for (size_t i = 0; i < components.size(); ++i) {
                result += components[i] * other.components[i];
            }
            return result;
        }
        
        Vector cross(const Vector& other) const {
            if (components.size() != 3 || other.components.size() != 3) {
                return Vector(3);
            }
            
            Vector result(3);
            result.components[0] = components[1] * other.components[2] - components[2] * other.components[1];
            result.components[1] = components[2] * other.components[0] - components[0] * other.components[2];
            result.components[2] = components[0] * other.components[1] - components[1] * other.components[0];
            return result;
        }
    };
    
    // Complex numbers
    struct Complex {
        double real, imag;
        
        Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}
        
        Complex add(const Complex& other) const {
            return Complex(real + other.real, imag + other.imag);
        }
        
        Complex multiply(const Complex& other) const {
            return Complex(real * other.real - imag * other.imag,
                          real * other.imag + imag * other.real);
        }
        
        Complex divide(const Complex& other) const {
            double denominator = other.real * other.real + other.imag * other.imag;
            if (denominator == 0.0) return Complex(0, 0);
            
            return Complex((real * other.real + imag * other.imag) / denominator,
                          (imag * other.real - real * other.imag) / denominator);
        }
        
        double magnitude() const {
            return sqrt(real * real + imag * imag);
        }
        
        Complex conjugate() const {
            return Complex(real, -imag);
        }
        
        Complex power(int n) const {
            if (n == 0) return Complex(1, 0);
            if (n < 0) return divide(Complex(1, 0), power(-n));
            
            Complex result = *this;
            for (int i = 1; i < n; ++i) {
                result = result.multiply(*this);
            }
            return result;
        }
    };
    
    // Statistics
    struct Statistics {
        std::vector<double> data;
        
        void addValue(double value) {
            data.push_back(value);
        }
        
        double mean() const {
            if (data.empty()) return 0.0;
            
            double sum = 0.0;
            for (double value : data) {
                sum += value;
            }
            return sum / data.size();
        }
        
        double median() const {
            if (data.empty()) return 0.0;
            
            std::vector<double> sorted = data;
            std::sort(sorted.begin(), sorted.end());
            
            size_t n = sorted.size();
            if (n % 2 == 0) {
                return (sorted[n/2 - 1] + sorted[n/2]) / 2.0;
            } else {
                return sorted[n/2];
            }
        }
        
        double standardDeviation() const {
            if (data.size() < 2) return 0.0;
            
            double m = mean();
            double sum = 0.0;
            for (double value : data) {
                sum += (value - m) * (value - m);
            }
            return sqrt(sum / (data.size() - 1));
        }
        
        double variance() const {
            double sd = standardDeviation();
            return sd * sd;
        }
        
        double percentile(double p) const {
            if (data.empty()) return 0.0;
            
            std::vector<double> sorted = data;
            std::sort(sorted.begin(), sorted.end());
            
            size_t index = static_cast<size_t>(p * (sorted.size() - 1) / 100.0);
            return sorted[std::min(index, sorted.size() - 1)];
        }
    };
    
    // Calculus
    struct Calculus {
        static double derivative(std::function<double(double)> f, double x, double h = 1e-6) {
            return (f(x + h) - f(x - h)) / (2 * h);
        }
        
        static double integral(std::function<double(double)> f, double a, double b, int n = 1000) {
            double h = (b - a) / n;
            double sum = 0.0;
            
            for (int i = 0; i <= n; ++i) {
                double x = a + i * h;
                double weight = (i == 0 || i == n) ? 1.0 : (i % 2 == 0) ? 2.0 : 4.0;
                sum += weight * f(x);
            }
            
            return sum * h / 3.0; // Simpson's rule
        }
        
        static double solveNewton(std::function<double(double)> f, std::function<double(double)> df, 
                                 double x0, double tolerance = 1e-6, int maxIterations = 100) {
            double x = x0;
            
            for (int i = 0; i < maxIterations; ++i) {
                double fx = f(x);
                double dfx = df(x);
                
                if (fabs(dfx) < tolerance) break; // Derivative too small
                
                double xNew = x - fx / dfx;
                
                if (fabs(xNew - x) < tolerance) return xNew;
                
                x = xNew;
            }
            
            return x; // Return best approximation
        }
    };
    
    MathEngine() {}
    
    Matrix* createMatrix(int rows, int cols, const std::vector<double>& values) {
        return new Matrix(rows, cols, values);
    }
    
    Vector* createVector(const std::vector<double>& components) {
        return new Vector(components);
    }
    
    Complex* createComplex(double real, double imag) {
        return new Complex(real, imag);
    }
    
    Statistics* createStatistics() {
        return new Statistics();
    }
};

// ============================================================
// Register mathematics natives
// ============================================================
void registerNativeMathematics(const std::shared_ptr<Environment>& globals) {

    static MathEngine mathEngine;
    
    // Math object with methods
    auto math = std::make_shared<ClawHashMap>();
    
    // Create matrix
    math->set("createMatrix", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            int rows = static_cast<int>(asNumber(args[0]));
            int cols = static_cast<int>(asNumber(args[1]));
            auto valuesArray = asArray(args[2]);
            
            std::vector<double> values;
            for (size_t i = 0; i < valuesArray->size(); ++i) {
                values.push_back(asNumber(valuesArray->get(i)));
            }
            
            auto matrix = mathEngine.createMatrix(rows, cols, values);
            auto matrixMap = std::make_shared<ClawHashMap>();
            matrixMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(matrix)));
            matrixMap->set("rows", numberToValue(rows));
            matrixMap->set("cols", numberToValue(cols));
            
            return hashMapValue(matrixMap);
        },
        "math.createMatrix"
    )));
    
    // Create vector
    math->set("createVector", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto componentsArray = asArray(args[0]);
            
            std::vector<double> components;
            for (size_t i = 0; i < componentsArray->size(); ++i) {
                components.push_back(asNumber(componentsArray->get(i)));
            }
            
            auto vector = mathEngine.createVector(components);
            auto vectorMap = std::make_shared<ClawHashMap>();
            vectorMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(vector)));
            vectorMap->set("size", numberToValue(components.size()));
            vectorMap->set("magnitude", numberToValue(vector->magnitude()));
            
            return hashMapValue(vectorMap);
        },
        "math.createVector"
    )));
    
    // Create complex number
    math->set("createComplex", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            double real = asNumber(args[0]);
            double imag = asNumber(args[1]);
            
            auto complex = mathEngine.createComplex(real, imag);
            auto complexMap = std::make_shared<ClawHashMap>();
            complexMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(complex)));
            complexMap->set("real", numberToValue(real));
            complexMap->set("imag", numberToValue(imag));
            complexMap->set("magnitude", numberToValue(complex->magnitude()));
            
            return hashMapValue(complexMap);
        },
        "math.createComplex"
    )));
    
    // Create statistics
    math->set("createStatistics", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto stats = mathEngine.createStatistics();
            auto statsMap = std::make_shared<ClawHashMap>();
            statsMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(stats)));
            
            return hashMapValue(statsMap);
        },
        "math.createStatistics"
    )));
    
    // Advanced math functions
    math->set("factorial", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int n = static_cast<int>(asNumber(args[0]));
            if (n < 0) return numberToValue(0.0);
            
            double result = 1.0;
            for (int i = 2; i <= n; ++i) {
                result *= i;
            }
            return numberToValue(result);
        },
        "math.factorial"
    )));
    
    math->set("fibonacci", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int n = static_cast<int>(asNumber(args[0]));
            if (n < 0) return numberToValue(0.0);
            if (n == 0) return numberToValue(0.0);
            if (n == 1) return numberToValue(1.0);
            
            double a = 0.0, b = 1.0;
            for (int i = 2; i <= n; ++i) {
                double temp = a + b;
                a = b;
                b = temp;
            }
            return numberToValue(b);
        },
        "math.fibonacci"
    )));
    
    globals->define("math", hashMapValue(math));
    
    // Convenience functions
    globals->define("factorial", callableValue(std::make_shared<NativeFunction>(
        1,
        [math](const std::vector<Value>& args) -> Value {
            return math->get("factorial")->call(*nullptr, args);
        },
        "factorial"
    )));
    
    globals->define("fibonacci", callableValue(std::make_shared<NativeFunction>(
        1,
        [math](const std::vector<Value>& args) -> Value {
            return math->get("fibonacci")->call(*nullptr, args);
        },
        "fibonacci"
    )));
}

} // namespace claw
