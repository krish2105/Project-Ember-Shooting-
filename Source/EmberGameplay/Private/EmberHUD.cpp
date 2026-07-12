#include "EmberHUD.h"
#include "EmberCharacter.h"
#include "EmberHealthComponent.h"
#include "EmberWeaponComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AEmberHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
    const FLinearColor CrosshairColor = FLinearColor(0.95f, 0.45f, 0.08f, 1.0f);
    DrawLine(Center.X - 8.0f, Center.Y, Center.X - 2.0f, Center.Y, CrosshairColor, 2.0f);
    DrawLine(Center.X + 2.0f, Center.Y, Center.X + 8.0f, Center.Y, CrosshairColor, 2.0f);
    DrawLine(Center.X, Center.Y - 8.0f, Center.X, Center.Y - 2.0f, CrosshairColor, 2.0f);
    DrawLine(Center.X, Center.Y + 2.0f, Center.X, Center.Y + 8.0f, CrosshairColor, 2.0f);

    const APlayerController* PC = GetOwningPlayerController();
    const AEmberCharacter* Character = PC ? Cast<AEmberCharacter>(PC->GetPawn()) : nullptr;
    if (!Character) return;
    const UEmberHealthComponent* Health = Character->GetHealthComponent();
    const UEmberWeaponComponent* Weapon = Character->GetWeaponComponent();
    const FString Status = FString::Printf(TEXT("%s     HEALTH %03.0f     AMMO %02d / %03d"),
        Weapon ? *Weapon->GetWeaponDisplayName().ToString().ToUpper() : TEXT("UNARMED"),
        Health ? Health->GetHealth() : 0.0f,
        Weapon ? Weapon->GetMagazineAmmo() : 0,
        Weapon ? Weapon->GetReserveAmmo() : 0);
    DrawText(Status, FLinearColor::White, 40.0f, Canvas->ClipY - 70.0f, nullptr, 1.2f, false);
    const FString CombatState = Character->IsAiming()
        ? (Character->IsFiringInputHeld() ? TEXT("AIM • FIRING") : TEXT("AIM"))
        : (Character->IsFiringInputHeld() ? TEXT("FIRING") : TEXT("HIP"));
    DrawText(CombatState, FLinearColor(1.0f, 0.45f, 0.08f),
        Canvas->ClipX - 190.0f, Canvas->ClipY - 70.0f, nullptr, 1.0f, false);

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(this, TEXT("EmberEnemy"), Enemies);
    const FVector P = Character->GetActorLocation();
    FString Objective = TEXT("REACH THE WAREHOUSE");
    if (P.X < -43000.0f) Objective = TEXT("MOVE OUT FROM INSERTION");
    else if (P.X < -22000.0f) Objective = TEXT("CLEAR THE FIRST PATROL");
    else if (P.X < 9000.0f) Objective = TEXT("PUSH THROUGH THE WAREHOUSE");
    else if (P.X < 18000.0f) Objective = TEXT("CHECKPOINT REACHED — TAKE THE CRANE ROUTE");
    else if (P.X < 28000.0f) Objective = TEXT("CLEAR THE CONTAINER APPROACH");
    else if (Enemies.Num() > 2) Objective = FString::Printf(TEXT("SECURE THE HARBOR — %d HOSTILES REMAIN"), Enemies.Num());
    else Objective = TEXT("EXTRACT AT THE SOUTHEAST WATERFRONT");
    if (Enemies.Num() <= 2 && FVector::DistSquared2D(P, FVector(43000.0f, -38000.0f, P.Z)) < FMath::Square(3500.0f))
        Objective = TEXT("MISSION COMPLETE — EMBERS AT THE WATERLINE");
    DrawText(Objective, FLinearColor(0.95f, 0.65f, 0.15f), 40.0f, 40.0f, nullptr, 1.15f, false);
    DrawText(TEXT("WASD / STICKS MOVE   RMB / LT AIM   LMB / RT FIRE   R / X RELOAD   Q / DPAD-R SHOULDER   1-6 WEAPONS   ESC / MENU PAUSE"),
        FLinearColor(0.7f, 0.75f, 0.8f), 40.0f, Canvas->ClipY - 35.0f, nullptr, 0.75f, false);

    if (UGameplayStatics::IsGamePaused(this))
    {
        DrawRect(FLinearColor(0.01f, 0.015f, 0.025f, 0.82f), 0, 0, Canvas->ClipX, Canvas->ClipY);
        DrawText(TEXT("PROJECT EMBER — PAUSED"), FLinearColor::White, Center.X - 180.0f, Center.Y - 30.0f, nullptr, 1.5f, false);
        DrawText(TEXT("Press ESC or MENU to resume"), FLinearColor(0.95f, 0.45f, 0.08f), Center.X - 145.0f, Center.Y + 20.0f, nullptr, 1.0f, false);
    }
}
