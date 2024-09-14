#ifndef ACTIVATION_FNCS_H
#define ACTIVATION_FNCS_H

#include <cmath>
#include <string>


enum ActivationType{
    RELU,
    SIGMOID
};

const std::string ACTIVATION_NAMES[] = {
    "ReLU",
    "Sigmoid"
};

class ActivationFn{
    public:
        ~ActivationFn(){};

        virtual double forward(const double input) = 0;
        virtual double backward(const double input) = 0;

        std::string name(){return ACTIVATION_NAMES[fn_type];}
        ActivationType type(){return fn_type;}
    protected:
        ActivationType fn_type;
};

class ReLU : public ActivationFn{
    public:
        double forward(const double input){
            return std::max(0.0, input);
        }
        double backward(const double input){
            return input > 0 ? 1 : 0;
        }
        
    private:
        ActivationType fn_type = ActivationType::RELU;
};

class Sigmoid : public ActivationFn{
    public:
        double forward(const double input){
            return 1 / (1 + std::exp(-input));
        }
        double backward(const double input){
            return input * (1 - input);
        }
    private:
        ActivationType fn_type = ActivationType::SIGMOID;
};

#endif // ACTIVATION_FNCS_H