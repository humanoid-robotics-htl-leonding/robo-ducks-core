import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoTextComponent } from './nao-text.component';

describe('NaoTextComponent', () => {
  let component: NaoTextComponent;
  let fixture: ComponentFixture<NaoTextComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoTextComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoTextComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
