################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/pca/embed.cpp \
../src/pca/embedppca.cpp 

OBJS += \
./src/pca/embed.o \
./src/pca/embedppca.o 

CPP_DEPS += \
./src/pca/embed.d \
./src/pca/embedppca.d 


# Each subdirectory must supply rules for building sources it contributes
src/pca/%.o: ../src/pca/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: C++ Compiler'
	$(CXX) -I"pch" -D_DEBUG -I"$(PROJ_PATH)\inc" -O0 -g3 -Wall -c -fmessage-length=0 $(TC_COMPILER_MISC) -fPIE --sysroot="$(SBI_SYSROOT)" -mthumb -I@system_includes -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


