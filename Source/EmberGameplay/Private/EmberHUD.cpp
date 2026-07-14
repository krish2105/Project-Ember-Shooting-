#include "EmberHUD.h"

#include "EmberArmorComponent.h"
#include "EmberCharacter.h"
#include "EmberHealthComponent.h"
#include "EmberWeaponComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace EmberHUDStyle
{
    const FLinearColor Panel(0.01f, 0.035f, 0.055f, 0.84f);
    const FLinearColor PanelLine(0.02f, 0.65f, 0.72f, 0.82f);
    const FLinearColor Cyan(0.05f, 0.92f, 0.92f, 1.0f);
    const FLinearColor Blue(0.08f, 0.48f, 0.95f, 1.0f);
    const FLinearColor Orange(1.0f, 0.43f, 0.08f, 1.0f);
    const FLinearColor Red(1.0f, 0.08f, 0.03f, 1.0f);
    const FLinearColor Muted(0.55f, 0.72f, 0.76f, 1.0f);
}

void AEmberHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    using namespace EmberHUDStyle;
    const float Scale = FMath::Max(0.65f, FMath::Min(Canvas->ClipX / 1920.0f, Canvas->ClipY / 1080.0f));
    const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
    const APlayerController* PC = GetOwningPlayerController();
    const AEmberCharacter* Character = PC ? Cast<AEmberCharacter>(PC->GetPawn()) : nullptr;
    if (!Character) return;

    const UEmberHealthComponent* Health = Character->GetHealthComponent();
    const UEmberArmorComponent* Armor = Character->GetArmorComponent();
    const UEmberWeaponComponent* Weapon = Character->GetWeaponComponent();
    const float HealthRatio = Health ? FMath::Clamp(Health->GetHealth() / FMath::Max(1.0f, Health->GetMaxHealth()), 0.0f, 1.0f) : 0.0f;
    const float ArmorRatio = Armor ? FMath::Clamp(Armor->GetArmor() / 100.0f, 0.0f, 1.0f) : 0.0f;
    const bool bReloading = Weapon && Weapon->IsReloading();

    const auto PanelBox = [this, Scale](float X, float Y, float W, float H)
    {
        DrawRect(EmberHUDStyle::Panel, X, Y, W, H);
        DrawLine(X, Y, X + W, Y, EmberHUDStyle::PanelLine, 2.0f * Scale);
        DrawLine(X, Y + H, X + W, Y + H, FLinearColor(0.01f, 0.24f, 0.3f, 0.75f), Scale);
    };
    const auto Bar = [this, Scale](float X, float Y, float W, float H, float Ratio, const FLinearColor& Color)
    {
        DrawRect(FLinearColor(0.015f, 0.06f, 0.075f, 0.92f), X, Y, W, H);
        DrawRect(Color, X + 2.0f * Scale, Y + 2.0f * Scale,
            FMath::Max(0.0f, (W - 4.0f * Scale) * Ratio), H - 4.0f * Scale);
    };

    const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    if (CachedEnemies.IsEmpty() || Now >= NextEnemyCacheRefresh)
    {
        TArray<AActor*> FoundEnemies;
        UGameplayStatics::GetAllActorsWithTag(this, TEXT("EmberEnemy"), FoundEnemies);
        CachedEnemies.Reset(FoundEnemies.Num());
        for (AActor* Enemy : FoundEnemies) CachedEnemies.Add(Enemy);
        NextEnemyCacheRefresh = Now + 1.0;
    }

    int32 LivingEnemies = 0;
    for (const TWeakObjectPtr<AActor>& Entry : CachedEnemies)
    {
        const AActor* Enemy = Entry.Get();
        if (!Enemy) continue;
        const UEmberHealthComponent* EnemyHealth = Enemy->FindComponentByClass<UEmberHealthComponent>();
        if (!EnemyHealth || !EnemyHealth->IsDead()) ++LivingEnemies;
    }

    const FVector P = Character->GetActorLocation();
    FString Objective = TEXT("REACH THE WAREHOUSE");
    FString Phase = TEXT("HARBOR INFILTRATION");
    if (P.X < -43000.0f) Objective = TEXT("MOVE OUT FROM INSERTION");
    else if (P.X < -22000.0f) Objective = TEXT("CLEAR THE FIRST PATROL");
    else if (P.X < 9000.0f) { Objective = TEXT("PUSH THROUGH THE WAREHOUSE"); Phase = TEXT("WAREHOUSE BREACH"); }
    else if (P.X < 18000.0f) { Objective = TEXT("TAKE THE ELEVATED CRANE ROUTE"); Phase = TEXT("CHECKPOINT REACHED"); }
    else if (P.X < 28000.0f) { Objective = TEXT("CLEAR THE CONTAINER APPROACH"); Phase = TEXT("CONTAINER YARD"); }
    else if (LivingEnemies > 2) { Objective = FString::Printf(TEXT("SECURE THE HARBOR — %d HOSTILES"), LivingEnemies); Phase = TEXT("FINAL ENGAGEMENT"); }
    else { Objective = TEXT("EXTRACT AT THE SOUTHEAST WATERFRONT"); Phase = TEXT("EXTRACTION AVAILABLE"); }
    if (LivingEnemies <= 2 && FVector::DistSquared2D(P, FVector(43000.0f, -38000.0f, P.Z)) < FMath::Square(3500.0f))
        Objective = TEXT("MISSION COMPLETE — EMBERS AT THE WATERLINE");

    // Top-left: compact operator, survivability and weapon state.
    const float LeftX = 34.0f * Scale;
    const float TopY = 32.0f * Scale;
    const float LeftW = 330.0f * Scale;
    PanelBox(LeftX, TopY, LeftW, 154.0f * Scale);
    DrawText(TEXT("EMBER // HARBOR-01"), Cyan, LeftX + 18.0f * Scale, TopY + 12.0f * Scale, nullptr, 0.9f * Scale, false);
    DrawText(FString::Printf(TEXT("%03.0f"), Health ? Health->GetHealth() : 0.0f), FLinearColor::White,
        LeftX + 18.0f * Scale, TopY + 42.0f * Scale, nullptr, 1.3f * Scale, false);
    Bar(LeftX + 74.0f * Scale, TopY + 49.0f * Scale, 232.0f * Scale, 12.0f * Scale, HealthRatio, Cyan);
    DrawText(TEXT("ARMOR"), Muted, LeftX + 18.0f * Scale, TopY + 76.0f * Scale, nullptr, 0.7f * Scale, false);
    Bar(LeftX + 74.0f * Scale, TopY + 82.0f * Scale, 232.0f * Scale, 9.0f * Scale, ArmorRatio, Blue);
    DrawText(FString::Printf(TEXT("%02d / %03d"), Weapon ? Weapon->GetMagazineAmmo() : 0, Weapon ? Weapon->GetReserveAmmo() : 0),
        FLinearColor::White, LeftX + 18.0f * Scale, TopY + 108.0f * Scale, nullptr, 1.0f * Scale, false);
    DrawText(Weapon ? Weapon->GetWeaponDisplayName().ToString().ToUpper() : TEXT("UNARMED"), Orange,
        LeftX + 132.0f * Scale, TopY + 112.0f * Scale, nullptr, 0.78f * Scale, false);

    DrawText(FString::Printf(TEXT("HOSTILES LEFT  %02d"), LivingEnemies), LivingEnemies > 0 ? Orange : Cyan,
        LeftX, TopY + 172.0f * Scale, nullptr, 0.9f * Scale, false);

    // Top-right: mission phase and objective, aligned like a tactical directive card.
    const float MissionW = 430.0f * Scale;
    const float MissionX = Canvas->ClipX - MissionW - 34.0f * Scale;
    PanelBox(MissionX, TopY, MissionW, 92.0f * Scale);
    DrawText(Phase, Orange, MissionX + 18.0f * Scale, TopY + 12.0f * Scale, nullptr, 0.83f * Scale, false);
    DrawText(Objective, FLinearColor::White, MissionX + 18.0f * Scale, TopY + 47.0f * Scale, nullptr, 0.78f * Scale, false);

    // Bottom-right: local tactical radar. Enemy dots are transformed into the player's frame.
    const float RadarRadius = 94.0f * Scale;
    const FVector2D RadarCenter(Canvas->ClipX - 136.0f * Scale, Canvas->ClipY - 142.0f * Scale);
    DrawRect(Panel, RadarCenter.X - RadarRadius - 14.0f * Scale, RadarCenter.Y - RadarRadius - 28.0f * Scale,
        RadarRadius * 2.0f + 28.0f * Scale, RadarRadius * 2.0f + 48.0f * Scale);
    DrawText(TEXT("TACTICAL RADAR"), Cyan, RadarCenter.X - RadarRadius, RadarCenter.Y - RadarRadius - 22.0f * Scale,
        nullptr, 0.65f * Scale, false);
    for (int32 Ring = 1; Ring <= 3; ++Ring)
    {
        const float R = RadarRadius * Ring / 3.0f;
        const int32 Segments = 32;
        for (int32 I = 0; I < Segments; ++I)
        {
            const float A0 = TWO_PI * I / Segments;
            const float A1 = TWO_PI * (I + 1) / Segments;
            DrawLine(RadarCenter.X + FMath::Cos(A0) * R, RadarCenter.Y + FMath::Sin(A0) * R,
                RadarCenter.X + FMath::Cos(A1) * R, RadarCenter.Y + FMath::Sin(A1) * R,
                FLinearColor(0.02f, 0.55f, 0.6f, 0.45f), Scale);
        }
    }
    DrawLine(RadarCenter.X - RadarRadius, RadarCenter.Y, RadarCenter.X + RadarRadius, RadarCenter.Y, PanelLine, Scale);
    DrawLine(RadarCenter.X, RadarCenter.Y - RadarRadius, RadarCenter.X, RadarCenter.Y + RadarRadius, PanelLine, Scale);
    DrawRect(Cyan, RadarCenter.X - 3.0f * Scale, RadarCenter.Y - 5.0f * Scale, 6.0f * Scale, 10.0f * Scale);
    const FTransform PlayerTransform = Character->GetActorTransform();
    for (const TWeakObjectPtr<AActor>& Entry : CachedEnemies)
    {
        const AActor* Enemy = Entry.Get();
        if (!Enemy) continue;
        const UEmberHealthComponent* EnemyHealth = Enemy->FindComponentByClass<UEmberHealthComponent>();
        if (EnemyHealth && EnemyHealth->IsDead()) continue;
        const FVector Local = PlayerTransform.InverseTransformPosition(Enemy->GetActorLocation());
        FVector2D Dot(Local.Y, -Local.X);
        Dot *= RadarRadius / 7000.0f;
        if (Dot.Size() > RadarRadius - 5.0f * Scale) Dot = Dot.GetSafeNormal() * (RadarRadius - 5.0f * Scale);
        DrawRect(Red, RadarCenter.X + Dot.X - 3.0f * Scale, RadarCenter.Y + Dot.Y - 3.0f * Scale,
            6.0f * Scale, 6.0f * Scale);
    }

    // Bottom-center: weapon strip and deterministic reload timing.
    static const TCHAR* SlotNames[] = { TEXT("A4"), TEXT("C9"), TEXT("P12"), TEXT("D3"), TEXT("L5"), TEXT("S9") };
    const float StripY = Canvas->ClipY - 72.0f * Scale;
    const float SlotW = 54.0f * Scale;
    const float StripX = Center.X - SlotW * 3.0f;
    for (int32 I = 0; I < 6; ++I)
    {
        const bool bSelected = Character->GetCurrentWeaponIndex() == I;
        DrawRect(bSelected ? FLinearColor(0.02f, 0.5f, 0.56f, 0.9f) : Panel,
            StripX + I * SlotW, StripY, SlotW - 4.0f * Scale, 34.0f * Scale);
        DrawText(FString::Printf(TEXT("%d %s"), I + 1, SlotNames[I]), bSelected ? FLinearColor::White : Muted,
            StripX + (I * SlotW) + 7.0f * Scale, StripY + 8.0f * Scale, nullptr, 0.58f * Scale, false);
    }
    if (bReloading)
    {
        const float ReloadW = 320.0f * Scale;
        const float ReloadX = Center.X - ReloadW * 0.5f;
        DrawText(TEXT("RELOADING"), Orange, ReloadX, StripY - 42.0f * Scale, nullptr, 0.78f * Scale, false);
        Bar(ReloadX, StripY - 18.0f * Scale, ReloadW, 8.0f * Scale, Weapon->GetReloadProgress(), Orange);
    }

    // Responsive reticle and hit confirmation stay visually quiet outside combat.
    const bool bHostileTarget = Character->HasHostileUnderCrosshair();
    const FLinearColor CrosshairColor = bHostileTarget ? Red : (Character->IsAiming() ? Cyan : Orange);
    const float Gap = (Character->IsAiming() ? 5.0f : 10.0f) * Scale;
    const float Length = 8.0f * Scale;
    DrawLine(Center.X - Gap - Length, Center.Y, Center.X - Gap, Center.Y, CrosshairColor, 2.0f * Scale);
    DrawLine(Center.X + Gap, Center.Y, Center.X + Gap + Length, Center.Y, CrosshairColor, 2.0f * Scale);
    DrawLine(Center.X, Center.Y - Gap - Length, Center.X, Center.Y - Gap, CrosshairColor, 2.0f * Scale);
    DrawLine(Center.X, Center.Y + Gap, Center.X, Center.Y + Gap + Length, CrosshairColor, 2.0f * Scale);
    if (Character->ShouldShowHitMarker())
    {
        const float H = 12.0f * Scale;
        const float I = 4.0f * Scale;
        DrawLine(Center.X - H, Center.Y - H, Center.X - I, Center.Y - I, FLinearColor::White, 2.5f * Scale);
        DrawLine(Center.X + H, Center.Y - H, Center.X + I, Center.Y - I, FLinearColor::White, 2.5f * Scale);
        DrawLine(Center.X - H, Center.Y + H, Center.X - I, Center.Y + I, FLinearColor::White, 2.5f * Scale);
        DrawLine(Center.X + H, Center.Y + H, Center.X + I, Center.Y + I, FLinearColor::White, 2.5f * Scale);
    }

    DrawText(TEXT("WASD/STICKS MOVE  •  HOLD RMB/LT AIM  •  LMB/RT FIRE  •  R/X RELOAD  •  Q/DPAD-R SHOULDER  •  ESC/MENU PAUSE"),
        Muted, 34.0f * Scale, Canvas->ClipY - 26.0f * Scale, nullptr, 0.53f * Scale, false);

    if (UGameplayStatics::IsGamePaused(this))
    {
        DrawRect(FLinearColor(0.005f, 0.015f, 0.025f, 0.9f), 0, 0, Canvas->ClipX, Canvas->ClipY);
        const float PauseW = 520.0f * Scale;
        PanelBox(Center.X - PauseW * 0.5f, Center.Y - 105.0f * Scale, PauseW, 210.0f * Scale);
        DrawText(TEXT("PROJECT EMBER"), Cyan, Center.X - 142.0f * Scale, Center.Y - 72.0f * Scale, nullptr, 1.45f * Scale, false);
        DrawText(TEXT("TACTICAL PAUSE"), Orange, Center.X - 104.0f * Scale, Center.Y - 25.0f * Scale, nullptr, 0.9f * Scale, false);
        DrawText(TEXT("ESC / MENU   RESUME MISSION"), FLinearColor::White,
            Center.X - 142.0f * Scale, Center.Y + 35.0f * Scale, nullptr, 0.78f * Scale, false);
    }
}
