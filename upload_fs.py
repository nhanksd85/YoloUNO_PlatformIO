Import("env")

def after_upload(source, target, env):
    print(">>> Uploading SPIFFS data folder...")
    env.Execute("pio run --target uploadfs")

env.AddPostAction("upload", after_upload)