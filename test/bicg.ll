; ModuleID = 'bicg.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @kernel_bicg([38 x i32]* nocapture readonly %A, i32* nocapture %s, i32* nocapture %q, i32* nocapture readonly %p, i32* nocapture readonly %r) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc26, %entry
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc26 ]
  %arrayidx = getelementptr inbounds i32* %q, i64 %indvars.iv48
  store i32 0, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx7 = getelementptr inbounds i32* %r, i64 %indvars.iv48
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds i32* %s, i64 %indvars.iv
  %0 = load i32* %arrayidx5, align 4, !tbaa !1
  %1 = load i32* %arrayidx7, align 4, !tbaa !1
  %arrayidx11 = getelementptr inbounds [38 x i32]* %A, i64 %indvars.iv48, i64 %indvars.iv
  %2 = load i32* %arrayidx11, align 4, !tbaa !1
  %mul = mul nsw i32 %2, %1
  %add = add nsw i32 %mul, %0
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %3 = load i32* %arrayidx, align 4, !tbaa !1
  %4 = load i32* %arrayidx11, align 4, !tbaa !1
  %arrayidx21 = getelementptr inbounds i32* %p, i64 %indvars.iv
  %5 = load i32* %arrayidx21, align 4, !tbaa !1
  %mul22 = mul nsw i32 %5, %4
  %add23 = add nsw i32 %mul22, %3
  store i32 %add23, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 38
  br i1 %exitcond, label %for.inc26, label %for.body3

for.inc26:                                        ; preds = %for.body3
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 42
  br i1 %exitcond50, label %for.end28, label %for.body

for.end28:                                        ; preds = %for.inc26
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"int", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
