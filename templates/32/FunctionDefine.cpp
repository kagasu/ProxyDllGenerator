// {0}
__declspec(naked) void func_{1}()
{{
	DLLEXPORT({3}, "{0}");
	__asm {{ jmp functions[4 * {2}] }}
}}
