//char signal_stack[SIGSTKSZ];

//void signal_handler(int sig)
//{

//    printf("Received signal %d\n", sig);
////     mmap(stack - 4096, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
////     exit(0);
//}

//void stack_mgt_init(void)
//{
//    stack_t ss;
//    ss.ss_sp = signal_stack;
//    ss.ss_flags = 0;
//    ss.ss_size = SIGSTKSZ;

//    if (sigaltstack(&ss, NULL)) {
//        perror("sigaltstack");
//    }


//    struct sigaction sig;
//    bzero(&sig, sizeof(sig));
//    sig.sa_handler = signal_handler;
//    sig.sa_flags = SA_ONSTACK | SA_RESTART;
//    //     sig.sa_sigaction = NULL;
//    sigfillset(&sig.sa_mask);

//    if (sigaction(SIGSEGV, &sig, NULL) == -1) {
//        perror("signal");
//    }


//    size_t stack_size = 4096;
//    //     for (int i = 0; i < 10000; i ++) {
//    stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
//    if (!stack) {
//        printf("Stack allocation failed\n");
//    }
//}
