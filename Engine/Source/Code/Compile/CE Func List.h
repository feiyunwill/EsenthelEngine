static void $nop(PARAMS) { thread.func_code_pos+=Call::Size(0); }
static void $call$20func(PARAMS) { thread.func_code_pos+=Call::Size(1); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(),                          null,                          null); }
static void $call$20func$20result(PARAMS) { thread.func_code_pos+=Call::Size(2); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(),                          null, call.param(1).address(thread)); }
static void $call$20method(PARAMS) { thread.func_code_pos+=Call::Size(2); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(), call.param(1).address(thread),                          null); }
static void $call$20method$20result(PARAMS) { thread.func_code_pos+=Call::Size(3); thread.call(*(CodeEnvironment::FuncBody*)call.param(0).getInstructP(), call.param(0).getInstructI2(), call.param(1).address(thread), call.param(2).address(thread)); }
static void $set$20const$201(PARAMS) { *(byte *)(thread.func_stack+call.param(0).getInstructI()) = (byte &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }
static void $set$20const$202(PARAMS) { *(short*)(thread.func_stack+call.param(0).getInstructI()) = (short&)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }
static void $set$20const$204(PARAMS) { *(int  *)(thread.func_stack+call.param(0).getInstructI()) = (int  &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }
static void $set$20const$208(PARAMS) { *(long *)(thread.func_stack+call.param(0).getInstructI()) = (long &)call.param(1).raw; thread.func_code_pos+=Call::Size(2); }
static void $set$20const$20heap$201(PARAMS) { *(byte *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,byte ); thread.func_code_pos+=Call::Size(2); }
static void $set$20const$20heap$202(PARAMS) { *(short*)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,short); thread.func_code_pos+=Call::Size(2); }
static void $set$20const$20heap$204(PARAMS) { *(int  *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,int  ); thread.func_code_pos+=Call::Size(2); }
static void $set$20const$20heap$208(PARAMS) { *(long *)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,long ); thread.func_code_pos+=Call::Size(2); }
static void $set$20addr$20heap(PARAMS) { *(ptr*)(thread.heap      +call.param(0).getInstructI()) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }
static void $set$20addr$20stack(PARAMS) { *(ptr*)(thread.func_stack+call.param(0).getInstructI()) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }
static void $set$20addr$20result(PARAMS) { *(ptr*)(thread.func_result                            ) = call.param(0).address(thread); thread.func_code_pos+=Call::Size(1); }
static void $indirection(PARAMS) { *(ptr*)(thread.func_stack+call.param(0).getInstructI()) = PARAM(1,ptr); thread.func_code_pos+=Call::Size(2); }
static void $address$20of(PARAMS) { PARAM(0,ptr) = call.param(1).address(thread); thread.func_code_pos+=Call::Size(2); }
static void $offset$201s(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,sbyte ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$201u(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,byte  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$202s(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,short ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$202u(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,ushort) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$204s(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,int   ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$204u(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,uint  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$208s(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,long  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $offset$208u(PARAMS) { PARAM_REF(0) = (byte*)call.param(1).address(thread) + PARAM(2,ulong ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $inc$20ptr$202(PARAMS) { ++PARAM(0,short*); thread.func_code_pos+=Call::Size(1); }
static void $dec$20ptr$202(PARAMS) { --PARAM(0,short*); thread.func_code_pos+=Call::Size(1); }
static void $inc$20ptr$204(PARAMS) { ++PARAM(0,int  *); thread.func_code_pos+=Call::Size(1); }
static void $dec$20ptr$204(PARAMS) { --PARAM(0,int  *); thread.func_code_pos+=Call::Size(1); }
static void $inc$20ptr$208(PARAMS) { ++PARAM(0,long *); thread.func_code_pos+=Call::Size(1); }
static void $dec$20ptr$208(PARAMS) { --PARAM(0,long *); thread.func_code_pos+=Call::Size(1); }
static void $inc$20ptr$20n(PARAMS) {   PARAM(0,byte *) += call.param(1).getInstructI(); thread.func_code_pos+=Call::Size(2); }
static void $dec$20ptr$20n(PARAMS) {   PARAM(0,byte *) -= call.param(1).getInstructI(); thread.func_code_pos+=Call::Size(2); }
static void $add$20ptr$201s(PARAMS) { PARAM(0,byte*) += PARAM(1,sbyte ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$201u(PARAMS) { PARAM(0,byte*) += PARAM(1,byte  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$202s(PARAMS) { PARAM(0,byte*) += PARAM(1,short ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$202u(PARAMS) { PARAM(0,byte*) += PARAM(1,ushort) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$204s(PARAMS) { PARAM(0,byte*) += PARAM(1,int   ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$204u(PARAMS) { PARAM(0,byte*) += PARAM(1,uint  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$208s(PARAMS) { PARAM(0,byte*) += PARAM(1,long  ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $add$20ptr$208u(PARAMS) { PARAM(0,byte*) += PARAM(1,ulong ) * call.param(2).getInstructI(); thread.func_code_pos+=Call::Size(3); }
static void $ptr$20add$201s(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,sbyte ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$201u(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,byte  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$202s(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,short ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$202u(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,ushort) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$204s(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,int   ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$204u(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,uint  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$208s(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,long  ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20add$208u(PARAMS) { PARAM(0,ptr) = PARAM(1,byte*) + PARAM(2,ulong ) * call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $ptr$20diff(PARAMS) { PARAM(0,uintptr) = (PARAM(1,byte*) - PARAM(2,byte*)) / call.param(3).getInstructI(); thread.func_code_pos+=Call::Size(4); }
static void $goto(PARAMS) {                     thread.func_code_pos = (byte*)call.param(0).getInstructP(); }
static void $goto$20cond$201(PARAMS) { if( PARAM(1,byte )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$202(PARAMS) { if( PARAM(1,short)) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$204(PARAMS) { if( PARAM(1,int  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$208(PARAMS) { if( PARAM(1,long )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20f(PARAMS) { if( PARAM(1,flt  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20d(PARAMS) { if( PARAM(1,dbl  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$201(PARAMS) { if(!PARAM(1,byte )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$202(PARAMS) { if(!PARAM(1,short)) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$204(PARAMS) { if(!PARAM(1,int  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$208(PARAMS) { if(!PARAM(1,long )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$20f(PARAMS) { if(!PARAM(1,flt  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }
static void $goto$20cond$20n$20d(PARAMS) { if(!PARAM(1,dbl  )) thread.func_code_pos = (byte*)call.param(0).getInstructP(); else thread.func_code_pos+=Call::Size(2); }

