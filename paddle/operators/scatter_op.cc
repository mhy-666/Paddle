/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/operators/scatter_op.h"
#include "paddle/framework/ddim.h"

namespace paddle {
namespace operators {

class ScatterOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

 protected:
  void InferShape(const framework::InferShapeContext &ctx) const override {
    PADDLE_ENFORCE_EQ(ctx.Input<Tensor>("Index")->dims().size(), 1,
                      "Update Index should be 1-D.");
    PADDLE_ENFORCE_EQ(ctx.Input<Tensor>("Ref")->dims().size(),
                      ctx.Input<Tensor>("Updates")->dims().size(),
                      "Reference and Updates should have the same shape size");
    PADDLE_ENFORCE_EQ(ctx.Input<Tensor>("Updates")->dims()[0],
                      ctx.Input<Tensor>("Index")->dims()[0],
                      "Updates and Index should have same batch-size.");
    framework::DDim data_dim(ctx.Input<Tensor>("Updates")->dims());
    for (int i = 1; i < data_dim.size(); ++i)
      PADDLE_ENFORCE_EQ(data_dim[i], ctx.Input<Tensor>("Updates")->dims()[i]);
    ctx.Output<Tensor>("Out")->Resize(ctx.Input<Tensor>("Ref")->dims());
  }
};

class ScatterGradOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

 protected:
  void InferShape(const framework::InferShapeContext &ctx) const override {
    auto *dUpdates = ctx.Output<Tensor>(framework::GradVarName("Updates"));
    auto *Updates = ctx.Input<Tensor>("Updates");
    auto *dRef = ctx.Output<Tensor>(framework::GradVarName("Ref"));
    auto *Ref = ctx.Input<Tensor>("Ref");

    dRef->Resize(Ref->dims());
    dUpdates->Resize(Updates->dims());
  }
};

class ScatterOpMaker : public framework::OpProtoAndCheckerMaker {
 public:
  ScatterOpMaker(framework::OpProto *proto,
                 framework::OpAttrChecker *op_checker)
      : OpProtoAndCheckerMaker(proto, op_checker) {
    AddInput("Ref", "The source input of scatter op");
    AddInput("Index",
             "The index input of scatter op where Ref will be updated");
    AddInput("Updates", "The updated value of updates op");
    AddOutput("Out", "The output of add op");
    AddComment(R"DOC(
Scatter Operator by selecting from the first axis, 

Out = Ref
Out[Index] = Ref[Index] + Updates
)DOC");
  }
};
}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OP(scatter, ops::ScatterOp, ops::ScatterOpMaker, scatter_grad,
            ops::ScatterGradOp);
REGISTER_OP_CPU_KERNEL(scatter,
                       ops::ScatterOpKernel<paddle::platform::CPUPlace, float>);
REGISTER_OP_CPU_KERNEL(
    scatter_grad,
    ops::ScatterGradientOpKernel<paddle::platform::CPUPlace, float>);
