#include "AI/BTService_findNearestPlayer.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void UBTService_findNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	/* 引擎已在 SetOwner 里填好 AIOwner 成员（protected，子类可直接用），不要再声明同名局部变量以免遮蔽类成员报 C4458 */
	if (!AIOwner)
	{
		return;
	}

	/* 获取具体的敌人（谁在受到这个Controller控制）*/
	APawn* OwningPawn = AIOwner->GetPawn();
	if (!OwningPawn)
	{
		return;
	}

	/* 如果有Player标签，敌人就是目标，否则Player是目标 （友方单位也设置为了Player）*/
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");

	/* 拥有Target标签的AActor */
	TArray<AActor*> TargetActors;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, TargetActors);
	/* 找到离自己最近的一个敌人 */
	float ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;

	for (auto* TargetActor : TargetActors) {
		float Distance = OwningPawn->GetDistanceTo(TargetActor);
		if (Distance < ClosestDistance) {
			ClosestDistance = Distance;
			ClosestActor = TargetActor;
		}
	}

	/* 取黑板组件：C++ 节点必须直接用组件读写，不能调 UBTFunctionLibrary（后者仅限蓝图编译出的节点，否则触发 ensureAsRuntimeWarning 且不生效）*/
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	/* 设置黑板变量 */
	BlackboardComp->SetValueAsObject(TargetToFollowSelector.SelectedKeyName, ClosestActor);
	BlackboardComp->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, ClosestDistance);
}