#define EXIT_SS(INFO) printf("%s\n", INFO);
#define EXIT_SS_RETURN_NULL(INFO) printf("%s\n", INFO);\
								  return NULL;
#define EXIT_SS_RETURN_NEGATIVE(INFO) printf("%s\n", INFO);\
									  return -1;
#define JUDGE_ERROR(FUNCTION_RES, INFO) if(FUNCTION_RES < 0){\
											EXIT_SS(INFO)\
										}
#define JUDGE_ERROR_RETURN(FUNCTION_RES, INFO) if(FUNCTION_RES < 0){\
											EXIT_SS(INFO)\
											exit(1);\
										}
#define JUDGE_ERROR_RETURN_NULL(FUNCTION_RES, INFO) if(FUNCTION_RES < 0){\
											EXIT_SS(INFO)\
											exit(1);\
										}
#define JUDGE_ERROR_NULL_RETURN_NULL(FUNCTION_RES, INFO) if(FUNCTION_RES == NULL){\
											EXIT_SS(INFO)\
											exit(1);\
										}
#define JUDGE_ERROR_PTHREAD_ERROR(FUNCTION_RES, INFO) if(FUNCTION_RES != 0){\
											EXIT_SS(INFO)\
										}
#define JUDGE_ERROR_PTHREAD_ERROR_RETURN(FUNCTION_RES, INFO) if(FUNCTION_RES != 0){\
											EXIT_SS(INFO)\
											exit(1);\
										}
#define CONNECT_ERROR_JUDGE_NULL(FUNCTION_RES, INFO, FD) if(FUNCTION_RES == NULL){\
														write(FD, INFO, strlen(INFO));\
														exit(1);\
													}
#define CONNECT_ERROR_JUDGE_NEGATIVE(FUNCTION_RES, INFO, FD) if(FUNCTION_RES < 0){\
														write(FD, INFO, strlen(INFO));\
														exit(1);\
													}

