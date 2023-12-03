module common {

    namespace common {

        export function getPromptInstance(): Prompt {
            return new Prompt();
        }

        class Prompt {
            public showAlert() {
                alert("thhis is a test");
            }
        }
    }
}