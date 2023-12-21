void system_init();
void enterDeepSleep(int waitsec);
inline void deepPowerDown();
inline void IJB_sleep();
inline static void IJB_reset();

int getSleepFlag() {
	return 0;
}
void system_init() {
}
void enterDeepSleep(int waitsec) {
}
inline void deepPowerDown() {
}
inline void IJB_sleep() {
}
inline static void IJB_reset() {
}
int IJB_wait(int n, int active) { // if stop ret 1
	return 0;
}

void ichigojam_main(void);

int main() {
	ichigojam_main();
	return 0;
}
void _start() {
	main();
}
