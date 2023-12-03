using Microsoft.AspNetCore.Components;
using Microsoft.JSInterop;

namespace TimeLine.Components.Pages;

/// <summary>
/// https://svgjs.dev/docs/3.0/
/// </summary>
public partial class Home : ComponentBase
{
    [Inject]
    IJSRuntime JS { get; set; }

    protected override void OnInitialized()
    {
        //var promptObject = JSRuntime.InvokeAsync<IJSObjectReference>("common.getPromptInstance");
        base.OnInitialized();
    }
    async void AAA()
    {
        //try
        //{
        //    var promptObject = await JS.InvokeAsync<IJSObjectReference>("common.getPromptInstance");
        //    await promptObject.InvokeVoidAsync("showAlert");
        //}
        //catch(Exception ex)
        //{
        //}

        try
        {
            var module = await JS.InvokeAsync<IJSObjectReference>("import", "./ts/file.js");
            var promptObject = await module.InvokeAsync<IJSObjectReference>("getPromptInstance");
            //await promptObject.InvokeVoidAsync("showAlert");
            await module.InvokeVoidAsync("draw", "#ssvvgg");
        }
        catch(Exception ex)
        {
        }    
        
        try
        {
            var moduleSvg = await JS.InvokeAsync<IJSObjectReference>("import", "./svg.js/svg.js");
            var module = await JS.InvokeAsync<IJSObjectReference>("import", "./ts/file.js");
            await module.InvokeAsync<IJSObjectReference>("draw","ssvvgg");
            //await promptObject.InvokeVoidAsync("showAlert");
        }
        catch(Exception ex)
        {
        }
    }

}