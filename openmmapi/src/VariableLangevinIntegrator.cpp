/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2008-2009 Stanford University and the Authors.      *
 * Authors: Peter Eastman                                                     *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include "openmm/VariableLangevinIntegrator.h"
#include "openmm/Context.h"
#include "openmm/internal/ContextImpl.h"
#include "openmm/kernels.h"
#include <limits>
#include <string>

using namespace OpenMM;
using std::string;
using std::vector;

VariableLangevinIntegrator::VariableLangevinIntegrator(double temperature, double frictionCoeff, double errorTol) {
    setTemperature(temperature);
    setFriction(frictionCoeff);
    setErrorTolerance(errorTol);
    setConstraintTolerance(1e-4);
    setRandomNumberSeed((int) time(NULL));
}

void VariableLangevinIntegrator::initialize(ContextImpl& contextRef) {
    context = &contextRef;
    kernel = context->getPlatform().createKernel(IntegrateVariableLangevinStepKernel::Name(), contextRef);
    dynamic_cast<IntegrateVariableLangevinStepKernel&>(kernel.getImpl()).initialize(contextRef.getSystem(), *this);
}

vector<string> VariableLangevinIntegrator::getKernelNames() {
    std::vector<std::string> names;
    names.push_back(IntegrateVariableLangevinStepKernel::Name());
    return names;
}

void VariableLangevinIntegrator::step(int steps) {
    for (int i = 0; i < steps; ++i) {
        context->updateContextState();
        context->calcForces();
        dynamic_cast<IntegrateVariableLangevinStepKernel&>(kernel.getImpl()).execute(*context, *this, std::numeric_limits<double>::infinity());
    }
}

void VariableLangevinIntegrator::stepTo(double time) {
    while (time > context->getTime()) {
        context->updateContextState();
        context->calcForces();
        dynamic_cast<IntegrateVariableLangevinStepKernel&>(kernel.getImpl()).execute(*context, *this, time);
    }
}
