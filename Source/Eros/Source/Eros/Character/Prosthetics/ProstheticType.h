#pragma once

/* Enum for prosthetics and their sockets. */
UENUM()
enum class EProstheticType : uint8
{
	PT_Arm		UMETA(DisplayName = Arm),
	PT_Leg		UMETA(DisplayName = Leg)
};