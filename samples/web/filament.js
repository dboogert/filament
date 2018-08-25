let files = [];

function canvas_render() {
    _render();
    window.requestAnimationFrame(canvas_render);
}

// This is called as soon as the wasm has finished JIT compilation.
Module.postRun = function() {

    // Create the WebGL 2.0 context.
    let canvas = document.getElementById('filament-canvas');
    let ctx = GL.createContext(canvas, {
        majorVersion: 2,
        minorVersion: 0,
        antialias: false,
        depth: false
    });
    GL.makeContextCurrent(ctx);

    // Call into C++.
    _launch();

    canvas_resize();
    window.addEventListener("resize", canvas_resize);

    // Start the requestAnimationFrame loop.
    canvas_render();
}

// This is typically called BEFORE the wasm has finished JIT compiling.
// The only useful thing to do here is start loading assets.
window.onload = async function() {
}

function canvas_resize(){
    let canvas = document.getElementById('filament-canvas');
    let pr = window.devicePixelRatio;
    let w = window.innerWidth;
    let h = window.innerHeight;
    canvas.width = (w * pr) | 0;
    canvas.height = (h * pr) | 0;
    _resize((w * pr) | 0, (h * pr) | 0 );
}

function file_load(url) {
    let promise = new Promise((success, failure) => {
        fetch(url).then(resp => {
            resp.arrayBuffer().then(arr => success(arr));
        });
    });
    return promise;
}

// Texture loading method snarfed from Aransentin/wasmdemo
let texctx = document.createElement('canvas').getContext('2d');
function texture_load(url) {
    let promise = new Promise((success, failure) => {
        let img = new Image();
        img.src = url;
        img.onerror = failure;
        img.onload = () => {
            texctx.canvas.width = img.width;
            texctx.canvas.height = img.height;
            texctx.width = img.width;
            texctx.height = img.height;
            texctx.drawImage(img, 0, 0);
            let imgd = texctx.getImageData(0, 0, img.width, img.height);
            success(new Uint8Array(imgd.data.buffer));
        };
    });
    return promise;
}
