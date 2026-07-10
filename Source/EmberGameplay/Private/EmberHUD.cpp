#include "EmberHUD.h"
#include "EmberCharacter.h"
#include "EmberHealthComponent.h"
#include "EmberWeaponComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

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
    const FString Status = FString::Printf(TEXT("HEALTH %03.0f     AMMO %02d / %03d"),
        Health ? Health->GetHealth() : 0.0f,
        Weapon ? Weapon->GetMagazineAmmo() : 0,
        Weapon ? Weapon->GetReserveAmmo() : 0);
    DrawText(Status, FLinearColor::White, 40.0f, Canvas->ClipY - 70.0f, nullptr, 1.2f, false);
}
