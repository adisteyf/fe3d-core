## Backends in fe3d
backend is important part of mods and realisations for **fe3d core**.

### Filenames
> filename structure:
> - `feA-backendB.c` _(.so/.dylib/.dll)_
>
> where:
>  - **A**: first letter of realisation `(render,sound,physics,window,etc.)`
>  - **B**: backend full API name `(opengl,vulkan,physx,glfw,etc.)`
> 
> for instance:
>  - `fer-backend_opengl.c`

Also, **B MUST ACT** like a postfix. _(`FeRenderAPI fe_renderapi_version_debugblank(void)`)_
