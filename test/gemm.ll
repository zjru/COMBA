; ModuleID = 'gemm.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @kernel_gemm([25 x i32]* nocapture %C, [30 x i32]* nocapture readonly %A, [25 x i32]* nocapture readonly %B) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc32, %entry
  %indvars.iv60 = phi i64 [ 0, %entry ], [ %indvars.iv.next61, %for.inc32 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [25 x i32]* %C, i64 %indvars.iv60, i64 %indvars.iv
  %0 = load i32* %arrayidx5, align 4, !tbaa !1
  %mul = shl nsw i32 %0, 1
  store i32 %mul, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 25
  br i1 %exitcond, label %for.cond9.preheader, label %for.body3

for.cond9.preheader:                              ; preds = %for.body3, %for.inc29
  %indvars.iv57 = phi i64 [ %indvars.iv.next58, %for.inc29 ], [ 0, %for.body3 ]
  %arrayidx15 = getelementptr inbounds [30 x i32]* %A, i64 %indvars.iv60, i64 %indvars.iv57
  br label %for.body11

for.body11:                                       ; preds = %for.body11, %for.cond9.preheader
  %indvars.iv54 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next55, %for.body11 ]
  %1 = load i32* %arrayidx15, align 4, !tbaa !1
  %mul16 = mul nsw i32 %1, 3
  %arrayidx20 = getelementptr inbounds [25 x i32]* %B, i64 %indvars.iv57, i64 %indvars.iv54
  %2 = load i32* %arrayidx20, align 4, !tbaa !1
  %mul21 = mul nsw i32 %mul16, %2
  %arrayidx25 = getelementptr inbounds [25 x i32]* %C, i64 %indvars.iv60, i64 %indvars.iv54
  %3 = load i32* %arrayidx25, align 4, !tbaa !1
  %add = add nsw i32 %3, %mul21
  store i32 %add, i32* %arrayidx25, align 4, !tbaa !1
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next55, 25
  br i1 %exitcond56, label %for.inc29, label %for.body11

for.inc29:                                        ; preds = %for.body11
  %indvars.iv.next58 = add nuw nsw i64 %indvars.iv57, 1
  %exitcond59 = icmp eq i64 %indvars.iv.next58, 30
  br i1 %exitcond59, label %for.inc32, label %for.cond9.preheader

for.inc32:                                        ; preds = %for.inc29
  %indvars.iv.next61 = add nuw nsw i64 %indvars.iv60, 1
  %exitcond62 = icmp eq i64 %indvars.iv.next61, 20
  br i1 %exitcond62, label %for.end34, label %for.cond1.preheader

for.end34:                                        ; preds = %for.inc32
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"int", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
