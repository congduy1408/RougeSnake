(()=>{"use strict";

const TERRAIN=new Set(["ground","wall","gap","entrance","door"]),$=id=>document.getElementById(id),clone=o=>JSON.parse(JSON.stringify(o));

const R={canvas:$("tilemap-canvas"),wrap:$("canvas-wrap"),status:$("status-text"),summary:$("marker-summary"),name:$("map-name"),w:$("map-width"),h:$("map-height"),cell:$("cell-size"),bg:$("bg-color"),grid:$("grid-color"),show:$("show-grid"),tiles:$("tile-list"),layers:$("layer-list"),zoom:$("zoom-label"),file:$("import-file")};const C=R.canvas.getContext("2d");

let S={name:"untitled",width:20,height:10,cellSize:24,settings:{bgColor:"#ffffff",gridColor:"#000000",showGrid:true},tiles:[{name:"ground",ratio:100,color:"#ae8a5e"},{name:"wall",ratio:100,color:"#555555"},{name:"gap",ratio:100,color:"#151515"},{name:"enemy",ratio:100,color:"#ff2600"},{name:"item",ratio:100,color:"#ffcc00"}],layers:[],layer:0,tile:"ground",tool:"brush",zoom:1};let undo=[],redo=[],drag=false,rect=null,rectEnd=null,edit=-1,spaceHeld=false,pan=null;

const mkLayer=n=>({name:n,visible:true,cells:[]}),snap=()=>clone(S);S.layers=[boundaryLayer()];

function boundaryLayer(){let L=mkLayer("Layer 0");for(let y=0;y<S.height;y++)for(let x=0;x<S.width;x++)if(x===0||y===0||x===S.width-1||y===S.height-1)L.cells.push({name:"wall",x,y});return L}

function typing(e){return e.target?.isContentEditable||["INPUT","TEXTAREA","SELECT"].includes(e.target?.tagName)}

function zoomAt(value,clientX,clientY){let before=R.canvas.getBoundingClientRect(),next=Math.max(.25,Math.min(4,value));if(next===S.zoom)return;let x=(clientX-before.left)/before.width,y=(clientY-before.top)/before.height;S.zoom=next;draw();let after=R.canvas.getBoundingClientRect();R.wrap.scrollLeft+=after.left+x*after.width-clientX;R.wrap.scrollTop+=after.top+y*after.height-clientY}

function centerZoom(value){let r=R.wrap.getBoundingClientRect();zoomAt(value,r.left+R.wrap.clientWidth/2,r.top+R.wrap.clientHeight/2)}

function endPan(){if(pan&&R.wrap.hasPointerCapture(pan.id))R.wrap.releasePointerCapture(pan.id);pan=null;R.wrap.classList.remove("is-panning")}



function ci(L,x,y){return L.cells.findIndex(c=>c.x===x&&c.y===y)}function set(L,x,y,n){let i=ci(L,x,y);if(n==null){if(i>=0)L.cells.splice(i,1)}else if(i>=0)L.cells[i].name=n;else L.cells.push({name:n,x,y})}function get(L,x,y){let i=ci(L,x,y);return i<0?null:L.cells[i].name}

function terrain(x,y){let v=null;S.layers.forEach(L=>{let n=get(L,x,y);if(TERRAIN.has(n))v=n});return v}function top(name){let a=[];for(let y=0;y<S.height;y++)for(let x=0;x<S.width;x++)if(terrain(x,y)===name)a.push({x,y});return a}function edge(p){return p.x===0||p.y===0||p.x===S.width-1||p.y===S.height-1}

function validate(){let e=top("entrance"),d=top("door"),errors=[];if(e.length!==1)errors.push(`Need exactly 1 entrance (found ${e.length})`);if(d.length<1||d.length>3)errors.push(`Need 1–3 exits (found ${d.length})`);[...e,...d].forEach(p=>{if(!edge(p))errors.push(`Marker at (${p.x},${p.y}) must be on boundary`)});return{ok:!errors.length,e,d,errors}}

function checkpoint(){undo.push(snap());if(undo.length>100)undo.shift();redo=[];hist()}function hist(){$("undo-btn").disabled=!undo.length;$("redo-btn").disabled=!redo.length}

function render(){R.name.value=S.name;R.w.value=S.width;R.h.value=S.height;R.cell.value=S.cellSize;R.bg.value=S.settings.bgColor;R.grid.value=S.settings.gridColor;R.show.checked=S.settings.showGrid;tiles();layers();draw();summary();hist();document.querySelectorAll("[data-tool]").forEach(b=>b.classList.toggle("active",b.dataset.tool===S.tool))}

function tiles(){R.tiles.innerHTML="";S.tiles.forEach((t,i)=>{let b=document.createElement("button");b.className="tile-row"+(S.tile===t.name?" selected":"");b.innerHTML=`<span class="tile-swatch" style="background:${t.color}"></span><span>${t.name}</span><span>${t.ratio}%</span>`;b.onclick=()=>{S.tile=t.name;S.tool="brush";render()};R.tiles.append(b)})}

function layers(){R.layers.innerHTML="";[...S.layers].reverse().forEach((L,r)=>{let i=S.layers.length-1-r,row=document.createElement("div"),check=document.createElement("input"),b=document.createElement("button");row.className="layer-row"+(i===S.layer?" selected":"");check.type="checkbox";check.checked=L.visible!==false;check.setAttribute("aria-label",`Show layer ${L.name}`);check.title="Show/hide layer (editor only)";check.onchange=()=>{checkpoint();L.visible=check.checked;render()};b.type="button";b.textContent=`${i}: ${L.name}`;b.onclick=()=>{S.layer=i;render()};row.append(check,b);R.layers.append(row)})}

function col(n){if(n==="entrance")return"#377bd1";if(n==="door")return"#37a55a";return S.tiles.find(t=>t.name===n)?.color||"#b070c0"}function draw(){let z=S.cellSize*S.zoom,w=S.width*z,h=S.height*z;R.canvas.width=Math.round(w);R.canvas.height=Math.round(h);C.fillStyle=S.settings.bgColor;C.fillRect(0,0,w,h);S.layers.filter(L=>L.visible!==false).forEach(L=>L.cells.forEach(c=>{C.fillStyle=col(c.name);C.fillRect(c.x*z,c.y*z,z,z);C.fillStyle="#111";C.font=`${Math.max(8,z*.35)}px sans-serif`;C.textAlign="center";C.textBaseline="middle";C.fillText(c.name[0].toUpperCase(),(c.x+.5)*z,(c.y+.5)*z)}));if(S.settings.showGrid){C.strokeStyle=S.settings.gridColor;C.beginPath();for(let x=0;x<=S.width;x++){C.moveTo(x*z,0);C.lineTo(x*z,h)}for(let y=0;y<=S.height;y++){C.moveTo(0,y*z);C.lineTo(w,y*z)}C.stroke()}drawRectanglePreview(z);R.zoom.textContent=`${Math.round(S.zoom*100)}%`}

function drawRectanglePreview(z){

  if(!drag||S.tool!=="rectangular"||!rect||!rectEnd)return;

  let x=Math.min(rect.x,rectEnd.x),y=Math.min(rect.y,rectEnd.y),w=Math.abs(rect.x-rectEnd.x)+1,h=Math.abs(rect.y-rectEnd.y)+1;

  C.save();

  C.globalAlpha=.35;C.fillStyle=col(S.tile);C.fillRect(x*z,y*z,w*z,h*z);

  C.globalAlpha=1;C.lineWidth=3;C.strokeStyle="#111827";C.strokeRect(x*z+1.5,y*z+1.5,w*z-3,h*z-3);

  C.lineWidth=1.5;C.strokeStyle="#ffffff";C.setLineDash([6,4]);C.strokeRect(x*z+1.5,y*z+1.5,w*z-3,h*z-3);

  C.restore();

  status(`Rectangle: ${w} × ${h} cells · Release to paint`);

}

function summary(){let v=validate();R.summary.innerHTML=v.ok?`<div class="marker-card"><strong>Valid</strong><div>1 entrance · ${v.d.length} exit${v.d.length===1?"":"s"}</div></div>`:`<div class="marker-card"><strong>Needs attention</strong>${v.errors.map(x=>`<div>${x}</div>`).join("")}</div>`}

function p(ev){let r=R.canvas.getBoundingClientRect();return{x:Math.max(0,Math.min(S.width-1,Math.floor((ev.clientX-r.left)/(r.width/S.width)))),y:Math.max(0,Math.min(S.height-1,Math.floor((ev.clientY-r.top)/(r.height/S.height))))}}

function clearName(n){S.layers.forEach(L=>L.cells=L.cells.filter(c=>c.name!==n))}function paint(q){let L=S.layers[S.layer];if(S.tool==="eraser")set(L,q.x,q.y,null);else if(S.tool==="brush")set(L,q.x,q.y,S.tile);else if(S.tool==="place-entrance"){if(!edge(q)){status("Entrance must be on boundary");return}clearName("entrance");set(L,q.x,q.y,"entrance")}else if(S.tool==="place-exit"){if(!edge(q)){status("Exit must be on boundary");return}if(terrain(q.x,q.y)!=="door"&&top("door").length>=3){status("Maximum 3 exits");return}set(L,q.x,q.y,"door")}draw();summary()}function status(x){R.status.textContent=x}

function move(d){let i=S.layer,j=i+d;if(j<0||j>=S.layers.length)return;checkpoint();[S.layers[i],S.layers[j]]=[S.layers[j],S.layers[i]];S.layer=j;render()}

function save(){let v=validate();if(!v.ok){alert("Cannot save:\n- "+v.errors.join("\n- "));return}let out={version:2,name:S.name,size:{width:S.width,height:S.height},settings:clone(S.settings),tiles:clone(S.tiles),layers:S.layers.map((L,index)=>({index,name:L.name,visible:L.visible!==false,cells:clone(L.cells)}))},blob=new Blob([JSON.stringify(out,null,2)],{type:"application/json"}),a=document.createElement("a");a.href=URL.createObjectURL(blob);a.download=S.name+".json";a.click();setTimeout(()=>URL.revokeObjectURL(a.href),500);status("Saved")}

function load(j){if(!j?.name||!j?.size||!Array.isArray(j.layers))throw Error("Not a Rouge Snake room JSON");S.name=j.name;S.width=+j.size.width;S.height=+j.size.height;S.settings={bgColor:j.settings?.bgColor||"#fff",gridColor:j.settings?.gridColor||"#000",showGrid:j.settings?.showGrid!==false};S.tiles=Array.isArray(j.tiles)?j.tiles:S.tiles;S.layers=[...j.layers].sort((a,b)=>(a.index??0)-(b.index??0)).map((L,i)=>({name:L.name||`Layer ${i}`,visible:L.visible!==false,cells:Array.isArray(L.cells)?L.cells:[]}));if(!S.layers.length)S.layers=[mkLayer("Layer 0")];S.layer=0;S.tile=S.tiles[0]?.name||"ground";S.tool="brush"}

function bind(){

$("new-map-btn").onclick=()=>{let name=prompt("Map name",S.name);if(!name)return;let w=+prompt("Width",S.width),h=+prompt("Height",S.height);if(!Number.isInteger(w)||!Number.isInteger(h)||w<3||h<3){alert("Width/height must be integers >= 3");return}checkpoint();S.name=name.trim();S.width=w;S.height=h;S.layers=[boundaryLayer()];S.layer=0;render();status("New map created with boundary walls")};$("import-btn").onclick=()=>R.file.click();R.file.onchange=async()=>{try{load(JSON.parse(await R.file.files[0].text()));undo=[];redo=[];render();status("Opened") }catch(e){alert(e.message)}R.file.value=""};$("export-btn").onclick=save;

$("undo-btn").onclick=()=>{if(!undo.length)return;redo.push(snap());S=undo.pop();render()};$("redo-btn").onclick=()=>{if(!redo.length)return;undo.push(snap());S=redo.pop();render()};

R.name.onchange=()=>{S.name=R.name.value.trim()||"untitled"};R.cell.onchange=()=>{S.cellSize=Math.max(8,Math.min(64,+R.cell.value||24));draw()};R.bg.oninput=()=>{S.settings.bgColor=R.bg.value;draw()};R.grid.oninput=()=>{S.settings.gridColor=R.grid.value;draw()};R.show.onchange=()=>{S.settings.showGrid=R.show.checked;draw()};

document.querySelectorAll("[data-tool]").forEach(b=>b.onclick=()=>{S.tool=b.dataset.tool;render()});R.canvas.onpointerdown=e=>{if(spaceHeld||e.button!==0)return;if(S.layers[S.layer].visible===false){status("Show the active layer before painting");return}R.canvas.setPointerCapture(e.pointerId);checkpoint();drag=true;let q=p(e);if(S.tool==="rectangular"){rect=q;rectEnd=q;draw()}else paint(q)};R.canvas.onpointermove=e=>{if(!drag)return;if(S.tool==="rectangular"&&rect){rectEnd=p(e);draw()}else if(S.tool==="brush"||S.tool==="eraser")paint(p(e))};R.canvas.onpointerup=e=>{if(!drag)return;if(S.tool==="rectangular"&&rect){let q=p(e),L=S.layers[S.layer];for(let y=Math.min(rect.y,q.y);y<=Math.max(rect.y,q.y);y++)for(let x=Math.min(rect.x,q.x);x<=Math.max(rect.x,q.x);x++)set(L,x,y,S.tile);status(`Painted rectangle: ${Math.abs(rect.x-q.x)+1} × ${Math.abs(rect.y-q.y)+1} cells`)}drag=false;rect=null;rectEnd=null;if(R.canvas.hasPointerCapture(e.pointerId))R.canvas.releasePointerCapture(e.pointerId);render()};R.canvas.onpointercancel=()=>{drag=false;rect=null;rectEnd=null;status("Drawing cancelled");render()};

$("add-layer-btn").onclick=()=>{let n=prompt("Layer name",`Layer ${S.layers.length}`);if(!n)return;checkpoint();S.layers.push(mkLayer(n));S.layer=S.layers.length-1;render()};$("rename-layer-btn").onclick=()=>{let n=prompt("Layer name",S.layers[S.layer].name);if(n){checkpoint();S.layers[S.layer].name=n;render()}};$("remove-layer-btn").onclick=()=>{if(S.layers.length===1){alert("Keep at least one layer");return}checkpoint();S.layers.splice(S.layer,1);S.layer=Math.min(S.layer,S.layers.length-1);render()};$("layer-up-btn").onclick=()=>move(1);$("layer-down-btn").onclick=()=>move(-1);$("clear-layer-btn").onclick=()=>{checkpoint();S.layers[S.layer].cells=[];render()};

$("clear-entrance-btn").onclick=()=>{checkpoint();clearName("entrance");render()};$("clear-exits-btn").onclick=()=>{checkpoint();clearName("door");render()};

$("add-tile-btn").onclick=()=>tileDialog(-1);$("edit-tile-btn").onclick=()=>tileDialog(S.tiles.findIndex(t=>t.name===S.tile));$("remove-tile-btn").onclick=()=>{let i=S.tiles.findIndex(t=>t.name===S.tile);if(i<0)return;if(!confirm(`Remove ${S.tile}? Existing cells keep the name.`))return;checkpoint();S.tiles.splice(i,1);S.tile=S.tiles[0]?.name||"";render()};$("tile-editor-cancel-btn").onclick=()=>$("tile-editor").hidden=true;$("tile-editor-save-btn").onclick=()=>saveTile();$("tile-editor-color").oninput=()=>$("tile-editor-swatch").style.background=$("tile-editor-color").value;

$("zoom-in-btn").onclick=()=>centerZoom(S.zoom*1.2);$("zoom-out-btn").onclick=()=>centerZoom(S.zoom/1.2);$("zoom-reset-btn").onclick=()=>{centerZoom(1)};

R.wrap.addEventListener("wheel",e=>{e.preventDefault();if(drag||pan||!e.deltaY)return;zoomAt(S.zoom*(e.deltaY<0?1.2:1/1.2),e.clientX,e.clientY)},{passive:false});

R.wrap.addEventListener("pointerdown",e=>{if(!spaceHeld||e.button!==0||drag)return;e.preventDefault();pan={id:e.pointerId,x:e.clientX,y:e.clientY,left:R.wrap.scrollLeft,top:R.wrap.scrollTop};R.wrap.setPointerCapture(e.pointerId);R.wrap.classList.add("is-panning")});

R.wrap.addEventListener("pointermove",e=>{if(!pan||pan.id!==e.pointerId)return;R.wrap.scrollLeft=pan.left+pan.x-e.clientX;R.wrap.scrollTop=pan.top+pan.y-e.clientY});

R.wrap.addEventListener("pointerup",endPan);R.wrap.addEventListener("pointercancel",endPan);R.wrap.addEventListener("lostpointercapture",endPan);

window.onkeydown=e=>{if(typing(e))return;if(e.code==="Space"){e.preventDefault();spaceHeld=true;R.wrap.classList.add("pan-ready");return}if((e.ctrlKey||e.metaKey)&&e.key.toLowerCase()==="z"){e.preventDefault();(e.shiftKey?$("redo-btn"):$("undo-btn")).click()}if((e.ctrlKey||e.metaKey)&&e.key.toLowerCase()==="y"){e.preventDefault();$("redo-btn").click()}};

window.onkeyup=e=>{if(e.code==="Space"){spaceHeld=false;R.wrap.classList.remove("pan-ready");endPan()}};

window.addEventListener("blur",()=>{spaceHeld=false;let wasDrawing=drag;drag=false;rect=null;rectEnd=null;R.wrap.classList.remove("pan-ready");endPan();if(wasDrawing){status("Drawing cancelled");draw()}});}



function tileDialog(i){edit=i;let t=i>=0?S.tiles[i]:{name:"",ratio:100,color:"#808080"};$("tile-editor-title").textContent=i>=0?"Edit Tile":"Add Tile";$("tile-editor-name").value=t.name;$("tile-editor-id").parentElement.style.display="none";$("tile-editor-ratio").value=t.ratio;$("tile-editor-color").value=t.color;$("tile-editor-swatch").style.background=t.color;$("tile-editor").hidden=false}

function saveTile(){let n=$("tile-editor-name").value.trim().toLowerCase();if(!n)return alert("Tile needs a name");if(n==="entrance"||n==="door")return alert("entrance and door are reserved Room Maker names");if(S.tiles.some((t,i)=>t.name===n&&i!==edit))return alert("Tile name already exists");checkpoint();let t={name:n,ratio:Math.max(0,Math.min(100,+$("tile-editor-ratio").value||0)),color:$("tile-editor-color").value};if(edit<0)S.tiles.push(t);else{let old=S.tiles[edit].name;S.tiles[edit]=t;S.layers.forEach(L=>L.cells.forEach(c=>{if(c.name===old)c.name=n}));S.tile=n}$("tile-editor").hidden=true;render()}

bind();render();})();

