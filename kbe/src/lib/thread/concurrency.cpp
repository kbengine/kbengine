#include "concurrency.hpp"
namespace KBEngine{
static void NoOp() { }
void (*pMainThreadIdleStartFunc)() = &NoOp;
void (*pMainThreadIdleEndFunc)() = &NoOp;

}
