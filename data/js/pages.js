function show_page(page_div_id, page_title){
	let page_div = document.getElementById(page_div_id);
	page_div.style.opacity = 1.0;
	page_div.style.pointerEvents = "all";
	document.getElementById("header_logo").innerHTML = page_title;
}

function hide_page(page_div_id){
	let page_div = document.getElementById(page_div_id);
	page_div.style.opacity = 0.0;
	page_div.style.pointerEvents = "none";
	document.getElementById("header_logo").innerHTML = "emotiscope";
}