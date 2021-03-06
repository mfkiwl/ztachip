#include <math.h>
#include "../../software/target/base/types.h"
#include "../../software/target/base/tensor.h"
#include "../../software/target/base/graph.h"
#include "ma_add.h"

// Graph node for matrix addition

GraphNodeMaAdd::GraphNodeMaAdd() {
}

GraphNodeMaAdd::GraphNodeMaAdd(TENSOR *input1,TENSOR *input2,TENSOR *output) {
   Create(input1,input2,output);
}

GraphNodeMaAdd::~GraphNodeMaAdd() {
}

ZtaStatus GraphNodeMaAdd::Create(TENSOR *input1,TENSOR *input2,TENSOR *output) {
   m_input1=input1;
   m_input2=input2;
   m_output=output;
   return ZtaStatusOk;
}


ZtaStatus GraphNodeMaAdd::Verify() {
   m_func=ztahostGetExportFunction("ma_add");
   assert(m_func != 0);
   // Output tensor to have dimension as input tensor
   m_output->Clone(m_input1);
   return ZtaStatusOk;
}

// Send request to mcore 

ZtaStatus GraphNodeMaAdd::Schedule(int queue) {
   ztahostMsgqWriteInt(queue,m_func);
   ztahostMsgqWriteInt(queue,GetNextRequestId(queue));
   ztahostMsgqWritePointer(queue,(void *)m_input1->GetBuf());
   ztahostMsgqWritePointer(queue,(void *)m_input2->GetBuf());
   ztahostMsgqWritePointer(queue,(void *)m_output->GetBuf());
   ztahostMsgqWriteInt(queue,m_input1->GetBufLen());
   return ZtaStatusOk;
}
