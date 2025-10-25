Import("env")

env.Append(
    CCFLAGS=[
        "-mfloat-abi=hard", # or softfp
        "-mfpu=fpv4-sp-d16",
        "-g3"
    ],
    LINKFLAGS=[
        "-mfloat-abi=hard", # or softp
        "-mfpu=fpv4-sp-d16",
   ]
)
