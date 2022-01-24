# PAX docs: Shaders

Shading is PAX' flagship feature.
It allows you to quickly and easily make colorful shapes or to add images to them.
For certain uses, shaders are a handy way to save on memory.

There are two options for using shaders:
- Use a [built-in shader](#built-in-shaders)
- Make [your own shader](#making-your-own-shaders)

# Built-in shaders

Using built-in shaders is very easy.

As of now, there is only one built-in shader:
| returns      | name               | arguments          | description
| :----------- | :----------------- | :----------------- | :----------
| pax_shader_t | PAX_SHADER_TEXTURE | pax_but_t \*buffer | Apply an image to a shape.

Every [`pax_shade_`](drawing.md#shaded-drawing) method expects pax_shader_t \*,
so you need to use addrof:
```c
// You can use any shader you want here.
pax_shader_t my_shader = PAX_SHADER_TEXTURE(&my_texture_buffer);
// Use the addrof (&) operator if it's not a pointer just yet.
pax_shade_rect(&buffer, color, &my_shader, NULL, x, y, width, height);
```

# Making your own shaders

The more complicated way is to make your own shader.

First, you need to define your callback:
```c
pax_col_t my_shader_callback(pax_col_t tint, int x, int y, float u, float v, void *args) {
    // Inside this method, do your magic.
    // In this case, this adds a bit of a rainbow tint to your shapes.
    return pax_col_hsv(x / 50.0 * 255.0 + y / 150.0 * 255.0, 255, 255);
}
```

Then, you need to define your shader struct:
```c
// The callback_args value is passed straight to args in my_shader_callback.
// Here, alpha_promise_0 states that the shader won't always return full transparency when the tint color's alpha is 0.
// Likewise, as we always return full opacity, alpha_promise_255 states that we do always return full opacity when the color's alpha is 255.
// These promises are used for optimisation internally,
// So if their condition is technically speaking true (you might ignore the tint like here), the promise is true.
pax_shader_t my_shader = (pax_shader_t) {
    .callback          = my_shader_callback,
    .callback_args     = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true
};
```

At this point, you use it like any other shader:
```c
// Let's make a rainbow circle!
// Color can be anything, since our shader ignores it.
pax_shade_cirle(&buffer, -1, &my_shader, NULL, x, y, radius);
```
