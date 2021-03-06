
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winternl.h>
#include <detours.h>

#include <main/main.h>
#include <objects/lights.h>

/* ---------- globals */

static bool g_forced_color_enabled = false;
static real_rgb_color g_forced_color = { 0.0f, 1.0f, 0.0f };

/* ---------- prototypes */

static void __fastcall light_update_state__hook(long light_index);
static decltype(light_update_state__hook) *light_update_state = nullptr;

static __int64 __fastcall c_function_definition__evaluate_color__hook(void *function, double input, double range, real_rgb_color *out_color);
static decltype(c_function_definition__evaluate_color__hook) *c_function_definition__evaluate_color = nullptr;

static double __fastcall c_function_definition__evaluate_scalar__hook(void *function, double input, double range);
static decltype(c_function_definition__evaluate_scalar__hook) *c_function_definition__evaluate_scalar = nullptr;

/* ---------- public code */

void light_hooks_initialize()
{
	c_function_definition__evaluate_color = main_get_typed_module_address<decltype(c_function_definition__evaluate_color)>(0x43C1C0);
	c_function_definition__evaluate_scalar = main_get_typed_module_address<decltype(c_function_definition__evaluate_scalar)>(0x43C660);
	light_update_state = main_get_typed_module_address<decltype(light_update_state)>(0x5C1040);

	DetourAttach((PVOID *)&c_function_definition__evaluate_color, c_function_definition__evaluate_color__hook);
	DetourAttach((PVOID *)&c_function_definition__evaluate_scalar, c_function_definition__evaluate_scalar__hook);
	DetourAttach((PVOID *)&light_update_state, light_update_state__hook);
}

void light_hooks_dispose()
{
	DetourDetach((PVOID *)&light_update_state, light_update_state__hook);
	DetourDetach((PVOID *)&c_function_definition__evaluate_scalar, c_function_definition__evaluate_scalar__hook);
	DetourDetach((PVOID *)&c_function_definition__evaluate_color, c_function_definition__evaluate_color__hook);
}

void lights_set_forced_color(bool enabled, real red, real green, real blue)
{
	g_forced_color_enabled = enabled;

	g_forced_color.red = red;
	g_forced_color.green = green;
	g_forced_color.blue = blue;
}

/* ---------- private globals */

static bool g_is_updating_light_state = false;
static long g_updating_light_state_index = -1;

/* ---------- private code */

static __int64 __fastcall c_function_definition__evaluate_color__hook(void *function, double input, double range, real_rgb_color *out_color)
{
	auto result = c_function_definition__evaluate_color(function, input, range, out_color);

	if (g_is_updating_light_state && g_forced_color_enabled && out_color)
	{
		memcpy(out_color, &g_forced_color, sizeof(real_rgb_color));
	}

	return result;
}

static double __fastcall c_function_definition__evaluate_scalar__hook(void *function, double input, double range)
{
	return c_function_definition__evaluate_scalar(function, input, range);
}

static void __fastcall light_update_state__hook(long light_index)
{
	g_is_updating_light_state = true;
	g_updating_light_state_index = light_index;

	light_update_state(light_index);

	g_updating_light_state_index = -1;
	g_is_updating_light_state = false;
}
