# Example hello triangle with SDL3's GPU API, SPIRV and C23

While browsing the internet for an example C implementation of [the Vulkan Hello World tutorial](https://docs.vulkan.org/tutorial/latest/00_Introduction.html) (which is only presented with c++20 and GLFW), I came across a few example repositories, like [this one](https://github.com/lonelydevil/Vulkan-Triangle) or [this one](https://github.com/stevelittlefish/c_vulkan_sdl3) which I found really useful. However, it did not took me long to realize I was simply not ready to spend a few weeks trying to understand my first 1000 lines of Vulkan when all my lurking math exams were menacing my first semester at my univerity.
I thus decided it would be fun to try SDL3's new GPU API, which provides a thin abstraction layer to the lower/lowest graphics APIs like Vulkan, Metal, DX12 and the like.
I ended up coming across [this amazing tutorial](https://hamdy-elzanqali.medium.com/let-there-be-triangles-sdl-gpu-edition-bd82cf2ef615) which ended up working for me.
Now I'm happy to share my own implementation of it with the internet, if that can help somebody, just like some other small repositories did for me.

# TL;DR
You must have :
- GNU make (you can install it on Windows with [mingw](https://www.mingw-w64.org/))
- A C compiler which supports C23 (if you want to run the code without any adjustment)
- The glslang compiler (or any other glsl compiler if you're willing to tweak the Makefile just a little, it's small)

*Then :*
```
make run
```
*or if you want to disable debugging :*
```
make run OPT=true
```

The example code is pretty small, about 180 lines as I do my first commit, hope that can help you out.

## About The absolute sorcery I did for error handling...
I tweaked the tutorial a little in its syntax and tried to get it to be as small as possible. For this reason, I created a "*tiny*" helper header [(include/SDLerr_helper.h)](https://github.com/Souleymeine/example-hello-triangle-SDL3-GPU-API-GLSL-SPIRV-C23/blob/main/include/SDLerr_helper.h) which includes some macro abuse (yes), but ended up removing a ton of boilerplate while maintaining a verbose error handling. If you want to get rid of it, simply replace the macros `FAILON`, `ASSIORFAIL` and `DECLORFAIL` with regular statements/assignments/declarations.
What it does is :
- nothing if the given expression (SDL function in our case) returns succesfully (return true)
- return -1 and thus ends the program while printing an error message with filename and line if the function returned an error (false)
Maybe I should move it to another repo and call it "Crappy SDL3 error handling macro magic syntactic sugar" or smh.

Also, I use C23's #embed preprocessor directive to include compiled shaders into the binary, which removes quite a lot of code and handles everything at compiletime without any runtime overhead.

If you don't want to use C23, you can actually read the spirv compiled shaders with SDL at runtime, the tutorial mentioned above does that really simply.
And don't forget to replace `nullptr`'s with `NULL`'s.

