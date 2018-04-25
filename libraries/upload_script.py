from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

#env.Replace(UPLOADHEXCMD='"$UPLOADER" --uploader --flags')

# uncomment line below to see environment variables
print env.Dump()