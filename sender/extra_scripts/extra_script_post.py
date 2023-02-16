# Import("env")

# # this is printed BEFORE any action is taken
# print("ZH: Current CLI targets", "dupa")
# print("ZH: Current Build targets", "aqq")

# # this is printed AFTER built
# def post_program_action(source, target, env):
#     print("ZH: Program has been built!")
#     program_path = target[0].get_abspath()
#     print("ZH: Program path", program_path)
#     env.Execute("date")

# env.AddPostAction("$PROGPATH", post_program_action)
