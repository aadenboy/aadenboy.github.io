let out = ``;
let offset = 0;
let amount = 6;
document.querySelectorAll(".ytcpPostRowPostContentText").forEach((postText, postIndex) => {
    if (postIndex - offset >= amount || postIndex < offset) return;
    const row = postText.parentNode.parentNode.parentNode;
    const date = row.querySelector("div:has(~ .ytcpPostRowDateDescription)");
    const action = row.querySelector(".ytcpPostRowPostAction");
    const images = row.querySelectorAll(".ytcpPostMultiImageThumbnailImage");
    let imgstr = "";
    images.forEach((image) => {
       imgstr += `<img src="${image.src.replace(/=s.+/, "=s0")}"loading="lazy"/>`
    });
    out += `<tr><td>${imgstr}</td><td><pre>${postText.innerHTML.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")}</pre></td><td>${date.innerHTML}</td><td><a href="${"https://www.youtube.com/post/" + action.href.match(/\/post\/(.+?)\/edit/)[1]}">View</a></td></tr>`
});
console.log(out);
