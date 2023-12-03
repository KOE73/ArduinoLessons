var common;
(function (common_1) {
    let common;
    (function (common) {
        function getPromptInstance() {
            return new Prompt();
        }
        common.getPromptInstance = getPromptInstance;
        class Prompt {
            showAlert() {
                alert("thhis is a test");
            }
        }
    })(common || (common = {}));
})(common || (common = {}));
//# sourceMappingURL=file1.js.map