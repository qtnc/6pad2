#include<csignal>

void CSignal ( void(*sigf)(int) ) {
signal(SIGSEGV, sigf);
signal(SIGFPE, sigf);
signal(SIGILL, sigf);
}
