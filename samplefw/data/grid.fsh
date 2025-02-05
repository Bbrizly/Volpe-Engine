uniform vec4 color;

in vec4 v_color;

out vec4 PixelColor;

void main()
{
    PixelColor = color * v_color;
}