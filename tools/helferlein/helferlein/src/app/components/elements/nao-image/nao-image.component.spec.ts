import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoImageComponent } from './nao-image.component';

describe('NaoImageComponent', () => {
  let component: NaoImageComponent;
  let fixture: ComponentFixture<NaoImageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoImageComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoImageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
