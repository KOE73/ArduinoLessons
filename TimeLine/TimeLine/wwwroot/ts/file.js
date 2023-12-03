//namespace common {
import { SVG } from '../svg.js/svg.esm.js';
class Prompt {
    showAlert() {
        alert("thhis is a test3");
    }
}
export function getPromptInstance() {
    return new Prompt();
}
//export function getSVG(id: string): Object
//{
//    var draw = svgjs.SVG(id);
//    return draw;
//}
export function draw(id) {
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
//# sourceMappingURL=file.js.map