qsb --glsl 100es -o externalsampler.vert.qsb externalsampler.vert
qsb --glsl 100es -o externalsampler.frag.qsb externalsampler.frag
qsb -r glsl,100es,externalsampler_gles.frag externalsampler.frag.qsb
