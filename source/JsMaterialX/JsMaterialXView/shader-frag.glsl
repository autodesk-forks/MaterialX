precision mediump float;
precision mediump int;

uniform float time;

void main()	{
    gl_FragColor = vec4(sin(time), 0, 0, 1);
}