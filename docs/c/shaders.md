# PAX Docs: Shaders

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
// The functions wants a pointer to a pax_shader_t.
pax_shade_rect(&buffer, color, &my_shader, NULL, x, y, width, height);
```

# Making your own shaders

## Data types

First, you need some information about the data stored in `pax_shader_t`:

| type    | name              | description
| :------ | :---------------- | :----------
| uint8_t | schema_version    | Version ID of shader schema, current is 1.
| uint8_t | schema_complement | Bitwise NOT of `schema_version`.
| uint8_t | renderer_id       | Which way to render the shader, set to `PAX_RENDERER_ID_SWR`.
| void \* | promise_callback  | Called to determine which optimisations can apply.
| void \* | callback          | The function that actually determines the color of each pixel.
| void \* | callback_args     | Passed as `args` to `promise_callback` and `callback`.
| bool    | alpha_promise_0   | Backwards compat: Promise that input color with 0 alpha creates output color with 0 alpha.
| bool    | alpha_promise_255 | Backwards compat: Promise that input color with 255 alpha creates output color with 255 alpha.

There are a few macros which define flags for `promise_callback` to return:

| name                    | description
| :---------------------- | :----------
| PAX_PROMISE_OPAQUE      | The shader will only return opaque colors.
| PAX_PROMISE_INVISIBLE   | Don't bother drawing with these parameters.
| PAX_PROMISE_CUTCOUT     | The shader with either return opaque or fully transparent colors.
| PAX_PROMISE_IGNORE_UVS  | The shader ignores parameters `u` and `v`.
| PAX_PROMISE_IGNORE_BASE | The shader ignores the `existing` color parameter.

## Implementation

The callback is used to turn some arguments and co-ordinates into a color to draw.
The `tint` is the color passed to the `pax_shade_` functions.
Similarly, `existing` is the color currently in the pixel you're about to specify a new color for.

Parameters `u` and `v` are the `x` and `y` components texture co-ordinates respectively, each ranging from 0-1.
These parameters can be used to tell where on the shape you're drawing.

Finally, `args` is the value of `callback_args` from the `pax_shader_t`.

For shaders with `schema_verion` of 1, the following function is an example of `callback`:
```c
pax_col_t my_shader_callback(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    // Inside this method, do your magic.
    // In this case, this adds a bit of a rainbow tint to your shapes.
    return pax_col_hsv(x / 50.0 * 255.0 + y / 150.0 * 255.0, 255, 255);
}
```

Optionally, you can tell PAX promises about what the shader will do:
```c
unit64_t my_promise_callback(pax_buf_t *buf, pax_col_t tint, void *args) {
    uint64_t flags = 0;
    
    // Our shader will always return a fully opaque color.
    flags |= PAX_PROMISE_OPAQUE;
    
    // In addition, it doesn't use the UV parameters.
    flags |= PAX_PROMISE_IGNORE_UVS;
    
    // Finally, it also ignores the existing color.
    flags |= PAX_PROMISE_IGNORE_BASE;
    
    return flags;
}
```

Then, you need to define your shader struct:
```c
pax_shader_t my_shader = (pax_shader_t) {
    .schema_verion     = 1,
    .schema_complement = ~1,
    .renderer_id       = PAX_RENDERER_ID_SWR,
    .callback          = my_shader_callback,
    .promise_callback  = my_promise_callback,
    .callback_args     = NULL,
    .alpha_promise_0   = false,
    .alpha_promise_255 = true
};
```
If you opted not to create `my_promise_callback`, you need to set `promise_callback` to `NULL`.


At this point, you use it like any other shader:
```c
// Let's make a rainbow circle!
// Color can be anything, since our shader ignores it.
pax_shade_cirle(&buffer, -1, &my_shader, NULL, x, y, radius);
```
