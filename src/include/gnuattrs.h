#define DOES_NOT_WRITE_GLOBAL_DATA	__attribute__((pure))
#define NONNULL				__attribute__((nonnull))
#define NORETURN			__attribute__((noreturn))
#define NO_SIDE_EFFECT			__attribute__((const))
#define PRINTFLIKE(arg_no)		__attribute__((format(printf, arg_no, arg_no + 1)))
#define SW_INLINE			inline
