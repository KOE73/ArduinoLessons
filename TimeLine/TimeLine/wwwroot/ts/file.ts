//namespace common {
import { SVG, extend as SVGextend, Element as SVGElement } from '../svg.js/svg.esm.js'
class Prompt {
    public showAlert() {
        alert("thhis is a test3");
    }
}
export function getPromptInstance(): Prompt {
    return new Prompt();
}




//export function getSVG(id: string): Object
//{
//    var draw = svgjs.SVG(id);
//    return draw;
//}

export function draw(id: string) {

    //var draw = SVG(id);
    var draw = SVG().addTo(id).size(300, 300);
    draw.rect(100, 100)
        //.animate({
        //    duration: 2000,
        //    delay: 1000,
        //    when: 'now',
        //    swing: true,
        //    times: 5,
        //    wait: 200
        //})
        .fill('#f03').move(100, 100);
}
//}
