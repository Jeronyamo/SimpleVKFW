#include "./source/renderer.h"
#include "./source/simpleio.h"
#include "./source/redo/diffrender2D.h"
//#include "./source/autodiff.h"
//#include "./source/matrices.h"

int main() {
    bool circ_exp = 0;
    bool text_exp = 0;
    bool trig_exp = 1;
    bool acdc_exp = 0;
    bool task_exp = 0;


// not implemented
    if (circ_exp) {
        std::cout << "\nCircle example" << std::endl;
        Simple::Circle2D circ{Simple::Vec2(0.3f), 0.1f, Simple::ColorA(1.f, 0.f, 0.f, 1.f)};
        Simple::Renderer2D scene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 256, -1.f / 256), Simple::UVec2(256), {Simple::Layer2D({&circ})}};
        scene.render().saveAsPNG("./res.png");
    }
    if (circ_exp) {
        Simple::DiffCircle2D circd{Simple::Vec2(0.35f, 0.3f), 0.12f, {32,32}};
        Simple::DiffRenderer2D diffscene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 256, -1.f / 256), Simple::UVec2(256), {Simple::DiffLayer2D({&circd})}};
        diffscene.render("./res.png", "./res_circle/", 201);
    }


    if (text_exp) {
        std::cout << "\nTexture restoration example" << std::endl;
        Simple::DiffCircle2D circd{Simple::Vec2(0.75f), 0.25f, {1024, 1024}, false};
        Simple::DiffRenderer2D diffscene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 1024, -1.f / 1024), Simple::UVec2(1024), {Simple::DiffLayer2D({&circd})}};
        diffscene.render("./resTex.png", "./res_texture/", 61);
        Simple::Image4 *texture = &(((Simple::Texture2D*)circd.tex)->tex);
        for (unsigned i = 0u; i < texture->width * texture->height; ++i)
            texture->image[i].a = 1.f;
        texture->saveAsPNG("./res_texture/restoresTex.png");
    }


    if (trig_exp) {
        std::cout << "\nTriangle example" << std::endl;
        Simple::Triangle2D tri3{{0.34, 0.31}, {0.67, 0.41}, {0.47, 0.74}, Simple::ColorA(1.f, 0.f, 0.f, 1.f)};
        Simple::Renderer2D scene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 256, -1.f / 256), Simple::UVec2(256), {Simple::Layer2D({&tri3})}};
        scene.render().saveAsPNG("./resT.png");
    }
    if (trig_exp) {
        // exactly tri_iter0
        //Simple::DiffTriangleWAS2D tri{{0.34, 0.34}, {0.6, 0.4}, {0.57, 0.74},Simple::ColorA(1.f, 0.f, 0.f, 1.f)};

        // strangely small dB.y
        //Simple::DiffTriangleWAS2D tri{{0.4f, 0.31f}, {0.7f, 0.1f}, {0.47f, 0.74f},Simple::ColorA(1.f, 0.f, 0.f, 1.f)};

        // new
        Simple::DiffTriangleWAS2D tri{{0.34f, 0.31f}, {0.7f, 0.41f}, {0.47f, 0.74f},Simple::ColorA(1.f, 0.f, 0.f, 1.f)};

        Simple::DiffRenderer2D diffscene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 256, -1.f / 256), Simple::UVec2(256), {Simple::DiffLayer2D({&tri})}};
        diffscene.render("./tri_iter0.png", "./res_triangle/", 231);
    }

    if (false) {
        std::cout << "Triangle example" << std::endl;
        Simple::Triangle2D tri1{{0.34, 0.31}, {0.67, 0.41}, {0.47, 0.74}, Simple::ColorA(1.f, 0.f, 0.f, 1.f)};
        Simple::Triangle2D tri2{{0.34, 0.31}, {0.67, 0.41}, {0.47, 0.74}, Simple::ColorA(1.f, 0.f, 0.f, 1.f)};
        Simple::Triangle2D tri3{{0.34, 0.31}, {0.67, 0.41}, {0.47, 0.74}, Simple::ColorA(1.f, 0.f, 0.f, 1.f)};
        Simple::Renderer2D scene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 1024, -1.f / 1024), Simple::UVec2(1024), {Simple::Layer2D({&tri1, &tri2, &tri3})}};
        scene.render().saveAsPNG("./resTrig.png");
    }
// Task 2
    if (task_exp) {
        std::cout << "WAS prep" << std::endl;
        Simple::DiffTriangleWAS2D tri1{{0.07, 0.05}, {0.046, 0.47}, {0.46, 0.51}, Simple::ColorA(1.f, 0.f, 0.f, 1.f), {0.01f, 0.01f}, {0.01f, 0.99f}, {0.99f, 0.99f}};
        Simple::DiffTriangleWAS2D tri3{{0.6, 0.95}, {0.55, 0.51}, {0.9, 0.86}, {32, 32}, {0.01f, 0.99f}, {0.01f, 0.01f}, {0.99f, 0.99f}};
        Simple::DiffTriangleWAS2D tri2{{0.0, 0.55}, {0.097, 0.9}, {0.41, 0.95}, {32, 32}, {0.01f, 0.01f}, {0.01f, 0.99f}, {0.99f, 0.99f}};
        Simple::DiffCircle2D circd{Simple::Vec2(0.7f, 0.3f), 0.1f, Simple::ColorA(0.543f, 0.2232f, 0.42f, 1.f), false};
        Simple::DiffRenderer2D diffscene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 256, -1.f / 256), Simple::UVec2(256), {Simple::DiffLayer2D({&tri1, &tri2, &tri3, &circd})}};
        std::cout << "WAS start" << std::endl;
        diffscene.render("./01_reference.png", "./res_was/", 351);
    }

    if (acdc_exp) {
        Simple::DiffSDFImage sdf{{256, 256}};
        Simple::DiffSDFImage sdfSmol{{64, 64}};
        Simple::DiffRenderer2D diffscene{Simple::Vec2(0.f, 1.f), Simple::Vec2(1.f / 1024, -1.f / 1024), Simple::UVec2(1024), {Simple::DiffLayer2D({&sdf})}};
        diffscene.render("./ACDC_logo.jpg", "./res_sdf/", 61, 0.01f);
        //Simple::Image4 *texture = &(((Simple::Texture2D*)sdf.tex)->tex);
        //texture->saveAsPNG("./res_sdf/restoredSDF.png");

        diffscene.layers = {Simple::DiffLayer2D({&sdfSmol})};
        diffscene.render("./ACDC_logo.jpg", "./res_sdf_smol/", 61, 0.01f);
        //texture = &(((Simple::Texture2D*)sdfSmol.tex)->tex);
        //texture->saveAsPNG("./res_sdf_smol/restoredSDFsmol.png");
    }
    return 0;
}